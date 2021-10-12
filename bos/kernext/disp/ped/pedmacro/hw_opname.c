/* @(#)83       1.5  src/bos/kernext/disp/ped/pedmacro/hw_opname.c, pedmacro, bos411, 9428A410j 3/24/94 13:54:42 */

/*
 * COMPONENT_NAME: pedmacro
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  THIS PROGRAM WILL CONVERT THE OPCODES IN THE INPUT FILES TO SYMBOLIC
 *  NAMES.  THERE ARE SEVERAL TABLES IN THIS FILE WHICH ARE NECCESSARY TO
 *  REDUCE THE AMOUNT OF DATA STORAGE THAT WOULD BE USED IF ONE TABLE WAS
 *  USED.  THERE IS A TABLE FOR EACH OCCURENCE OF A DIFFERENT DIGIT IN THE
 *  FOURTH PLACE OF THE HEX OPCODES LOCATED IN hw_seops.h.
 *    THERE ARE ALSO SOME TEMPORARY VALUES WHICH ARE PRECEDED BY "TEMP " IN
 *  THERE NAME.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mid/hw_seops.h>

/*
 *  THE FOLLOWING CONSTANTS ARE THE MIN & MAX OPCODE VALUES
 *  WHEN ONLY THE RIGHTMOST THREE DIGITS ARE CONSIDERED 
 *  IN hw_seops.h.
 *   ex: If for all the 0x9nnn opcodes 0x9f23 is
 *       the largest one, then the MAX_0x9nnn_OPCODE
 *       would be at least 0xf23.
 */
#define  MAX_0x0nnn_OPCODE  0x03FF
#define  MAX_0x1nnn_OPCODE  0x10000000
#define  MAX_0x8nnn_OPCODE  0x8403
#define  MAX_0x9nnn_OPCODE  0x900F
#define  MAX_0xAnnn_OPCODE  0xA0F7
#define  MAX_0xFnnn_OPCODE  0xFFFF

#define  MIN_0x0nnn_OPCODE  0x0000
#define  MIN_0x1nnn_OPCODE  0x10000000
#define  MIN_0x8nnn_OPCODE  0x8008
#define  MIN_0x9nnn_OPCODE  0x9002
#define  MIN_0xAnnn_OPCODE  0xA001
#define  MIN_0xFnnn_OPCODE  0xF000

/*
 *  THE FOLLOWING VARIABLE ARE USED EXTERNALLY BY TED
 */
int      max_0x0nnn_OPCODE = MAX_0x0nnn_OPCODE;
int      max_0x1nnn_OPCODE = MAX_0x1nnn_OPCODE;
int      max_0x8nnn_OPCODE = MAX_0x8nnn_OPCODE;
int      max_0x9nnn_OPCODE = MAX_0x9nnn_OPCODE;
int      max_0xAnnn_OPCODE = MAX_0xAnnn_OPCODE;
int      max_0xFnnn_OPCODE = MAX_0xFnnn_OPCODE;

int      min_0x0nnn_OPCODE = MIN_0x0nnn_OPCODE;
int      min_0x1nnn_OPCODE = MIN_0x1nnn_OPCODE;
int      min_0x8nnn_OPCODE = MIN_0x8nnn_OPCODE;
int      min_0x9nnn_OPCODE = MIN_0x9nnn_OPCODE;
int      min_0xAnnn_OPCODE = MIN_0xAnnn_OPCODE;
int      min_0xFnnn_OPCODE = MIN_0xFnnn_OPCODE;


/*
 *  THE FOLLOWING ARE TABLES TO STORE POINTERS
 *  TO THE OPCODE NAME OF THE OPCODE VALUE.
 */
char *table0[MAX_0x0nnn_OPCODE+1-MIN_0x0nnn_OPCODE];
char *table1[MAX_0x1nnn_OPCODE+1-MIN_0x1nnn_OPCODE];
char *table8[MAX_0x8nnn_OPCODE+1-MIN_0x8nnn_OPCODE];
char *table9[MAX_0x9nnn_OPCODE+1-MIN_0x9nnn_OPCODE];
char *tableA[MAX_0xAnnn_OPCODE+1-MIN_0xAnnn_OPCODE];
char *tableF[MAX_0xFnnn_OPCODE+1-MIN_0xFnnn_OPCODE];

char errmsg[40];

char* opcode_name ( opcode )
int opcode;
{

   int
      index, val, max_index;

   static int
      initialized = 0;

   if ( !initialized )
   {
      init_opcode_table();
      initialized = 1;
   }

   opcode &= 0x0000ffff;
   val = (opcode & 0xf000) >> 12;

   switch ( val )
   {
      case 0x0:
         index = opcode - MIN_0x0nnn_OPCODE;
         max_index = MAX_0x0nnn_OPCODE - MIN_0x0nnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( table0[index] != 0 ) return ( table0[index] );
         break;

      case 0x1:
         index = opcode - MIN_0x1nnn_OPCODE;
         max_index = MAX_0x1nnn_OPCODE - MIN_0x1nnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( table1[index] != 0 ) return ( table1[index] );
         break;

      case 0x8:
         index = opcode - MIN_0x8nnn_OPCODE;
         max_index = MAX_0x8nnn_OPCODE - MIN_0x8nnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( table8[index] != 0 ) return ( table8[index] );
         break;

      case 0x9:
         index = opcode - MIN_0x9nnn_OPCODE;
         max_index = MAX_0x9nnn_OPCODE - MIN_0x9nnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( table9[index] != 0 ) return ( table9[index] );
         break;

      case 0xA:
         index = opcode - MIN_0xAnnn_OPCODE;
         max_index = MAX_0xAnnn_OPCODE - MIN_0xAnnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( tableA[index] != 0 ) return ( tableA[index] );
         break;

      case 0xF:
         index = opcode - MIN_0xFnnn_OPCODE;
         max_index = MAX_0xFnnn_OPCODE - MIN_0xFnnn_OPCODE;
         if ( index >= 0 && index <= max_index )
            if ( tableF[index] != 0 ) return ( tableF[index] );
         break;

   }
#ifdef MID_DD
   sprintf(errmsg,"UNKNOWN Opcode = %4X",opcode);
#else /* MID_DD */
   sprintf(errmsg,"UNKNOWN Opcode = %4.4X",opcode);
#endif /* MID_DD */
   return(errmsg);
}


/****************************************************************************/
/* INITIALIZE THE NAME TABLE ARRAYS.                                        */
/****************************************************************************/

init_opcode_table()
{
   register int opcode, offset;

   offset = MAX_0x0nnn_OPCODE - MIN_0x0nnn_OPCODE;
#ifdef debug
   printf ("Initialize 0x0 table from %x to %x: offset %x\n",
             MIN_0x0nnn_OPCODE, MAX_0x0nnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      table0[opcode] = 0;


   offset = MAX_0x1nnn_OPCODE - MIN_0x1nnn_OPCODE;
#ifdef debug
   printf ("Initialize 0x1 table from %x to %x: offset %x\n",
             MIN_0x1nnn_OPCODE, MAX_0x1nnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      table1[opcode] = 0;


   offset = MAX_0x8nnn_OPCODE - MIN_0x8nnn_OPCODE;
#ifdef debug
   printf ("Initialize 0x8 table from %x to %x: offset %x\n",
             MIN_0x8nnn_OPCODE, MAX_0x8nnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      table8[opcode] = 0;


   offset = MAX_0x9nnn_OPCODE - MIN_0x9nnn_OPCODE;
#ifdef debug
   printf ("Initialize 0x9 table from %x to %x: offset %x\n",
             MIN_0x9nnn_OPCODE, MAX_0x9nnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      table9[opcode] = 0;


   offset = MAX_0xAnnn_OPCODE - MIN_0xAnnn_OPCODE;
#ifdef debug
   printf ("Initialize 0xA table from %x to %x: offset %x\n",
             MIN_0xAnnn_OPCODE, MAX_0xAnnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      tableA[opcode] = 0;


   offset = MAX_0xFnnn_OPCODE - MIN_0xFnnn_OPCODE;
#ifdef debug
   printf ("Initialize 0xF table from %x to %x: offset %x\n",
             MIN_0xFnnn_OPCODE, MAX_0xFnnn_OPCODE, offset);
#endif

   for ( opcode=0; opcode<offset; opcode++ )
      tableF[opcode] = 0;




   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0x0nnn */
   /* *************************************************************** */

   offset = MIN_0x0nnn_OPCODE;
#ifdef debug
   printf ("Loading 0x0 table, offset: %x %d\n",offset,offset);
#endif

   table0[OP_INDEX_MASK-offset] = "INDEX_MASK";
   table0[OP_NO_OPERATION-offset] = "NO_OPERATION";
   table0[OP_SET_POLYLINE_INDEX-offset] = "SET_POLYLINE_INDEX";
   table0[OP_SET_POLYMARKER_INDEX-offset] = "SET_POLYMARKER_INDEX";
   table0[OP_SET_EDGE_INDEX-offset] = "SET_EDGE_INDEX";
   table0[OP_SET_INTERIOR_INDEX-offset] = "SET_INTERIOR_INDEX";
   table0[OP_SET_DEPTH_CUE_INDEX-offset] = "SET_DEPTH_CUE_INDEX";
   table0[OP_SET_COLOR_PROCESSING_INDEX-offset] = "SET_COLOR_PROCESSING_INDEX";
   table0[OP_SET_LINETYPE-offset] = "SET_LINETYPE";
   table0[OP_SET_LINEWIDTH_SCALE_FACTOR-offset] = "SET_LINEWIDTH_SCALE_FACTOR";
   table0[OP_SET_POLYLINE_COLOR_INDEX-offset] = "SET_POLYLINE_COLOR_INDEX";
   table0[OP_SET_MARKER_TYPE-offset] = "SET_MARKER_TYPE";
   table0[OP_SET_MARKER_SIZE_SCALE_FACTOR-offset] = "SET_MARKER_SIZE_SCALE_FACTOR";
   table0[OP_SET_POLYMARKER_COLOR_INDEX-offset] = "SET_POLYMARKER_COLOR_INDEX";
   table0[OP_SET_TEXT_PRECISION-offset] = "SET_TEXT_PRECISION";
   table0[OP_SET_CHARACTER_EXPANSION_FACTOR-offset] = "SET_CHARACTER_EXPANSION_FACTOR";
   table0[OP_SET_CHARACTER_SPACING-offset] = "SET_CHARACTER_SPACING";
   table0[OP_SET_TEXT_COLOR_INDEX-offset] = "SET_TEXT_COLOR_INDEX";
   table0[OP_SET_CHARACTER_HEIGHT-offset] = "SET_CHARACTER_HEIGHT";
   table0[OP_SET_CHARACTER_UP_VECTOR-offset] = "SET_CHARACTER_UP_VECTOR";
   table0[OP_SET_TEXT_PATH-offset] = "SET_TEXT_PATH";
   table0[OP_SET_CHARACTER_POSITIONING_MODE-offset] = "SET_CHARACTER_POSITIONING_MODE";
   table0[OP_SET_TEXT_ALIGNMENT-offset] = "SET_TEXT_ALIGNMENT";
   table0[OP_SET_INTERIOR_STYLE-offset] = "SET_INTERIOR_STYLE";
   table0[OP_SET_INTERIOR_STYLE_INDEX-offset] = "SET_INTERIOR_STYLE_INDEX";
   table0[OP_SET_INTERIOR_COLOR_INDEX-offset] = "SET_INTERIOR_COLOR_INDEX";
   table0[OP_SET_EDGE_FLAG-offset] = "SET_EDGE_FLAG";
   table0[OP_SET_EDGE_LINETYPE-offset] = "SET_EDGE_LINETYPE";
   table0[OP_SET_EDGE_COLOR_INDEX-offset] = "SET_EDGE_COLOR_INDEX";
   table0[OP_SET_EDGE_SCALE_FACTOR-offset] = "SET_EDGE_SCALE_FACTOR";
   table0[OP_SET_POLYLINE_END_TYPE-offset] = "SET_POLYLINE_END_TYPE";
   table0[OP_SET_ANNOTATION_STYLE-offset] = "SET_ANNOTATION_STYLE";
   table0[OP_SET_ANNOTATION_HEIGHT_SCALE_FACT-offset] = "SET_ANNOTATION_HEIGHT_SCALE_FACT";
   table0[OP_SET_ANNOTATION_HEIGHT-offset] = "SET_ANNOTATION_HEIGHT";
   table0[OP_SET_ANNOTATION_UP_VECTOR-offset] = "SET_ANNOTATION_UP_VECTOR";
   table0[OP_SET_ANNOTATION_PATH-offset] = "SET_ANNOTATION_PATH";
   table0[OP_SET_ANNOTATION_ALIGNMENT-offset] = "SET_ANNOTATION_ALIGNMENT";
   table0[OP_SET_CHARACTER_UP_AND_BASE_VECTOR-offset] = "SET_CHARACTER_UP_AND_BASE_VECTOR";
   table0[OP_SET_CHARACTER_LINE_SCALE_FACTOR-offset] = "SET_CHARACTER_LINE_SCALE_FACTOR";
   table0[OP_SET_POLYLINE_COLOR_DIRECT-offset] = "SET_POLYLINE_COLOR_DIRECT";
   table0[OP_SET_POLYMARKER_COLOR_DIRECT-offset] = "SET_POLYMARKER_COLOR_DIRECT";
   table0[OP_SET_TEXT_COLOR_DIRECT-offset] = "SET_TEXT_COLOR_DIRECT";
   table0[OP_SET_INTERIOR_COLOR_DIRECT-offset] = "SET_INTERIOR_COLOR_DIRECT";
   table0[OP_SET_EDGE_COLOR_DIRECT-offset] = "SET_EDGE_COLOR_DIRECT";
   table0[OP_SET_FRAME_BUFFER_MASK-offset] = "SET_FRAME_BUFFER_MASK";
   table0[OP_SET_FRAME_BUFFER_COMPARISON-offset] = "SET_FRAME_BUFFER_COMPARISON";
   table0[OP_SET_BACK_INTERIOR_COLOR_INDEX-offset] = "SET_BACK_INTERIOR_COLOR_INDEX";
   table0[OP_SET_BACK_INTERIOR_COLOR_DIRECT-offset] = "SET_BACK_INTERIOR_COLOR_DIRECT";
   table0[OP_SET_SPECULAR_COLOR_INDEX-offset] = "SET_SPECULAR_COLOR_INDEX";
   table0[OP_SET_SPECULAR_COLOR_DIRECT-offset] = "SET_SPECULAR_COLOR_DIRECT";
   table0[OP_SET_BACK_SPECULAR_COLOR_INDEX-offset] = "SET_BACK_SPECULAR_COLOR_INDEX";
   table0[OP_SET_BACK_SPECULAR_COLOR_DIRECT-offset] = "SET_BACK_SPECULAR_COLOR_DIRECT";
   table0[OP_SET_POLYGON_CULLING-offset] = "SET_POLYGON_CULLING";
   table0[OP_SET_SURFACE_PROPERTIES-offset] = "SET_SURFACE_PROPERTIES";
   table0[OP_SET_BACK_SURFACE_PROPERTIES-offset] = "SET_BACK_SURFACE_PROPERTIES";
   table0[OP_SET_FACE_DISTINGUISH_MODE-offset] = "SET_FACE_DISTINGUISH_MODE";
   table0[OP_SET_LIGHT_SOURCE_STATE-offset] = "SET_LIGHT_SOURCE_STATE";
   table0[OP_SET_HLHSR_IDENTIFIER-offset] = "SET_HLHSR_IDENTIFIER";
   table0[OP_SET_CURVE_APPROXIMATION_CRITERIA-offset] = "SET_CURVE_APPROXIMATION_CRITERIA";
   table0[OP_SET_POLYHEDRON_EDGE_CULLING-offset] = "SET_POLYHEDRON_EDGE_CULLING";
   table0[OP_SET_LIGHTING_CALCULATION_MODE-offset] = "SET_LIGHTING_CALCULATION_MODE";
   table0[OP_SET_ANTI_ALIASING_IDENTIFIER-offset] = "SET_ANTI_ALIASING_IDENTIFIER";
   table0[OP_SET_FACE_LIGHTING_METHOD-offset] = "SET_FACE_LIGHTING_METHOD";
   table0[OP_SET_Z_BUFFER_PROTECT_MASK-offset] = "SET_Z_BUFFER_PROTECT_MASK";
   table0[OP_SET_POLYLINE_SHADING_METHOD-offset] = "SET_POLYLINE_SHADING_METHOD";
   table0[OP_SET_MODELLING_TRANSFORMATION_3-offset] = "SET_MODELLING_TRANSFORMATION_3";
   table0[OP_SET_MODELLING_TRANSFORMATION_2-offset] = "SET_MODELLING_TRANSFORMATION_2";
   table0[OP_SET_GLOBAL_TRANSFORMATION_3-offset] = "SET_GLOBAL_TRANSFORMATION_3";
   table0[OP_SET_GLOBAL_TRANSFORMATION_2-offset] = "SET_GLOBAL_TRANSFORMATION_2";
   table0[OP_SET_VIEW_INDEX-offset] = "SET_VIEW_INDEX";
   table0[OP_SET_HIGHLIGHTING_COLOR_INDEX-offset] = "SET_HIGHLIGHTING_COLOR_INDEX";
   table0[OP_SET_HIGHLIGHTING_COLOR_DIRECT-offset] = "SET_HIGHLIGHTING_COLOR_DIRECT";
   table0[OP_ADD_CLASS_NAME_TO_SET-offset] = "ADD_CLASS_NAME_TO_SET";
   table0[OP_REMOVE_CLASS_NAME_FROM_SET-offset] = "REMOVE_CLASS_NAME_FROM_SET";
   table0[OP_INSERT_APPLICATION_DATA-offset] = "INSERT_APPLICATION_DATA";
   table0[OP_EXECUTE_STRUCTURE-offset] = "EXECUTE_STRUCTURE";
   table0[OP_INSERT_LABEL-offset] = "INSERT_LABEL";
   table0[OP_SET_PICK_IDENTIFIER-offset] = "SET_PICK_IDENTIFIER";
   table0[OP_CONDITIONAL_EXECUTE_STRUCTURE-offset] = "CONDITIONAL_EXECUTE_STRUCTURE";
   table0[OP_NO_OPERATION_II-offset] = "NO_OPERATION_II";
   table0[OP_POLYLINE_3-offset] = "POLYLINE_3";
   table0[OP_POLYLINE_2-offset] = "POLYLINE_2";
   table0[OP_DISJOINT_POLYLINE_3-offset] = "DISJOINT_POLYLINE_3";
   table0[OP_DISJOINT_POLYLINE_2-offset] = "DISJOINT_POLYLINE_2";
   table0[OP_POLYMARKER_3-offset] = "POLYMARKER_3";
   table0[OP_POLYMARKER_2-offset] = "POLYMARKER_2";
   table0[OP_CIRCLE_2-offset] = "CIRCLE_2";
   table0[OP_CIRCULAR_ARC_2-offset] = "CIRCULAR_ARC_2";
   table0[OP_NON_UNIFORM_B_SPLINE_CURVE_3-offset] = "NON_UNIFORM_B_SPLINE_CURVE_3";
   table0[OP_NON_UNIFORM_B_SPLINE_CURVE_2-offset] = "NON_UNIFORM_B_SPLINE_CURVE_2";
   table0[OP_ELLIPSE_3-offset] = "ELLIPSE_3";
   table0[OP_ELLIPSE_2-offset] = "ELLIPSE_2";
   table0[OP_ELLIPTICAL_ARC_3-offset] = "ELLIPTICAL_ARC_3";
   table0[OP_ELLIPTICAL_ARC_2-offset] = "ELLIPTICAL_ARC_2";
   table0[OP_POLYGON_3-offset] = "POLYGON_3";
   table0[OP_POLYGON_2-offset] = "POLYGON_2";
   table0[OP_MARKER_GRID_3-offset] = "MARKER_GRID_3";
   table0[OP_MARKER_GRID_2-offset] = "MARKER_GRID_2";
   table0[OP_LINE_GRID_3-offset] = "LINE_GRID_3";
   table0[OP_LINE_GRID_2-offset] = "LINE_GRID_2";
   table0[OP_POLYGON_WITH_DATA_3-offset] = "POLYGON_WITH_DATA_3";
   table0[OP_POLYGON_WITH_DATA_2-offset] = "POLYGON_WITH_DATA_2";
   table0[OP_TRIANGLE_STRIP_3-offset] = "TRIANGLE_STRIP_3";
   table0[OP_COMPOSITE_FILL_AREA_2-offset] = "COMPOSITE_FILL_AREA_2";
   table0[OP_POLYHEDRON_EDGE-offset] = "POLYHEDRON_EDGE";
   table0[OP_CELL_ARRAY_3-offset] = "CELL_ARRAY_3";
   table0[OP_CELL_ARRAY_2-offset] = "CELL_ARRAY_2";
   table0[OP_POLYSPHERE-offset] = "POLYSPHERE";
   table0[OP_FILL_AREA_3-offset] = "FILL_AREA_3";
   table0[OP_FILL_AREA_2-offset] = "FILL_AREA_2";
   table0[OP_POLYLINE_SET_3_WITH_DATA-offset] = "POLYLINE_SET_3_WITH_DATA";
   table0[OP_V3F-offset] = "V3F";
   table0[OP_END_LINE-offset] = "END_LINE";
   table0[OP_END_POINT-offset] = "END_POINT";
   table0[OP_COLOR_INDEX-offset] = "COLOR_INDEX";
   table0[OP_C3I-offset] = "C3I";
   table0[OP_C3F-offset] = "C3F";
   table0[OP_N3F-offset] = "N3F";
   table0[OP_C3F_AD-offset] = "C3F_AD";
   table0[OP_C3F_AMBIENT-offset] = "C3F_AMBIENT";
   table0[OP_C3F_DIFFUSE-offset] = "C3F_DIFFUSE";
   table0[OP_C3F_EMISSION-offset] = "C3F_EMISSION";
   table0[OP_C3F_SPECULAR-offset] = "C3F_SPECULAR";
   table0[OP_MAT_STATE-offset] = "MAT_STATE";
   table0[OP_MAT_PROPERTIES-offset] = "MAT_PROPERTIES";
   table0[OP_BACK_MATERIAL_STATE_M1M-offset] = "BACK_MATERIAL_STATE_M1M";
   table0[OP_BACK_MATERIAL_PROPERTIES_M1M-offset] = "BACK_MATERIAL_PROPERTIES_M1M";
   table0[OP_V4F-offset] = "V4F";
   table0[OP_V3I-offset] = "V3I";


   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0x1nnn */
   /* *************************************************************** */

   offset = MIN_0x1nnn_OPCODE;
#ifdef debug
   printf ("Loading 0x1 table, offset: %x %d\n",offset,offset);
#endif

   table1[OP_UNDEFINED_OPCODE-offset] = "UNDEFINED_OPCODE";


   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0x8nnn */
   /* *************************************************************** */

   offset = MIN_0x8nnn_OPCODE;
#ifdef debug
   printf ("Loading 0x8 table, offset: %x %d\n",offset,offset);
#endif

   table8[OP_SET_LINETYPE_M1M-offset] = "SET_LINETYPE_M1M";
   table8[OP_SET_LINEWIDTH_SCALE_FACTOR_M1M-offset] = "SET_LINEWIDTH_SCALE_FACTOR_M1M";
   table8[OP_SET_INTERIOR_STYLE_M1M-offset] = "SET_INTERIOR_STYLE_M1M";
   table8[OP_SET_POLYLINE_END_TYPE_M1M-offset] = "SET_POLYLINE_END_TYPE_M1M";
   table8[OP_SET_POLYGON_CULLING_M1M-offset] = "SET_POLYGON_CULLING_M1M";
   table8[OP_SET_CURVE_APPROXIMATION_CRIT_M1M-offset] = "SET_CURVE_APPROXIMATION_CRIT_M1M";
   table8[OP_SET_SURFACE_APPROXIMATION_CR_M1M-offset] = "SET_SURFACE_APPROXIMATION_CR_M1M";
   table8[OP_SET_LIGHTING_CALCULATION_MOD_M1M-offset] = "SET_LIGHTING_CALCULATION_MOD_M1M";
   table8[OP_SET_TRIMMING_CURVE_APPROX_CR_M1M-offset] = "SET_TRIMMING_CURVE_APPROX_CR_M1M";
   table8[OP_SET_MODELLING_TRANSFORMATION_M1M-offset] = "SET_MODELLING_TRANSFORMATION_M1M";
   table8[OP_NON_UNIFORM_B_SPLINE_CURVE_3_M1M-offset] = "NON_UNIFORM_B_SPLINE_CURVE_3_M1M";
   table8[OP_NON_UNIFORM_B_SPLINE_SURFACE_M1M-offset] = "NON_UNIFORM_B_SPLINE_SURFACE_M1M";
   table8[OP_TRIMMED_NON_UNIFORM_B_SPLINE_M1M-offset] = "TRIMMED_NON_UNIFORM_B_SPLINE_M1M";
   table8[OP_SET_ACTIVE_FONT-offset] = "SET_ACTIVE_FONT";
   table8[OP_REMOVE_ACTIVE_FONT-offset] = "REMOVE_ACTIVE_FONT";
   table8[OP_DMA_QUEUE_DMA_REQUEST-offset] = "DMA_QUEUE_DMA_REQUEST";
   table8[OP_SET_WINDOW_PARAMETERS-offset] = "SET_WINDOW_PARAMETERS";
   table8[OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO-offset] = "ASSOC_WINDOW_WITH_COLOR_PAL_FIFO";
   table8[OP_TRANSPARENCY_AND_COLOR_COMPARE-offset] = "TRANSPARENCY_AND_COLOR_COMPARE";
   table8[OP_SYNCHRONIZE_ON_VERTICAL_RETRACE-offset] = "SYNCHRONIZE_ON_VERTICAL_RETRACE";
   table8[OP_SYNCHRONIZATION-offset] = "SYNCHRONIZATION";
   table8[OP_CLEAR_CONTROL_PLANES_FIFO-offset] = "CLEAR_CONTROL_PLANES_FIFO";
   table8[OP_SELECT_DRAWING_BIT_PLANES-offset] = "SELECT_DRAWING_BIT_PLANES";
   table8[OP_SET_BIT_PLANES_WRITE_MASKS-offset] = "SET_BIT_PLANES_WRITE_MASKS";
   table8[OP_SET_BIT_PLANES_DISPLAY_MASKS-offset] = "SET_BIT_PLANES_DISPLAY_MASKS";
   table8[OP_FRAME_BUFFER_CONTROL-offset] = "FRAME_BUFFER_CONTROL";
   table8[OP_SET_FRAME_BUFFER_SWAP_INTERVAL-offset] = "SET_FRAME_BUFFER_SWAP_INTERVAL";
   table8[OP_Z_BUFFER_CONTROL-offset] = "Z_BUFFER_CONTROL";
   table8[OP_CLEAR_BIT_PLANES-offset] = "CLEAR_BIT_PLANES";
   table8[OP_SET_GPM_COMPARE-offset] = "SET_GPM_COMPARE";
   table8[OP_SET_LOGICAL_OPERATION-offset] = "SET_LOGICAL_OPERATION";
   table8[OP_READ_PIXEL_DATA-offset] = "READ_PIXEL_DATA";
   table8[OP_WRITE_PIXEL_DATA-offset] = "WRITE_PIXEL_DATA";
   table8[OP_PIXEL_BLIT-offset] = "PIXEL_BLIT";
   table8[OP_ZOOM-offset] = "ZOOM";
   table8[OP_SET_FOREGROUND_PIXEL-offset] = "SET_FOREGROUND_PIXEL";
   table8[OP_SET_BACKGROUND_PIXEL-offset] = "SET_BACKGROUND_PIXEL";
   table8[OP_SET_LINE_ATTRIBUTES-offset] = "SET_LINE_ATTRIBUTES";
   table8[OP_SET_FILL_ATTRIBUTES-offset] = "SET_FILL_ATTRIBUTES";
   table8[OP_DEFINE_LINE_PATTERN-offset] = "DEFINE_LINE_PATTERN";
   table8[OP_DEFINE_TILE-offset] = "DEFINE_TILE";
   table8[OP_DEFINE_STIPPLE-offset] = "DEFINE_STIPPLE";
   table8[OP_SET_PATTERN_ORIGIN-offset] = "SET_PATTERN_ORIGIN";
   table8[OP_SET_2D_COLOR_MODE-offset] = "SET_2D_COLOR_MODE";
   table8[OP_SET_2D_CLIENT_CLIP-offset] = "SET_2D_CLIENT_CLIP";
   table8[OP_POLYPOINT-offset] = "POLYPOINT";
   table8[OP_POLYLINES-offset] = "POLYLINES";
   table8[OP_POLYSEGMENT-offset] = "POLYSEGMENT";
   table8[OP_POLYRECTANGLE-offset] = "POLYRECTANGLE";
   table8[OP_FILL_POLYGON-offset] = "FILL_POLYGON";
   table8[OP_FILL_POLYRECTANGLE-offset] = "FILL_POLYRECTANGLE";
   table8[OP_POLYTEXT_8-offset] = "POLYTEXT_8";
   table8[OP_POLYTEXT_16-offset] = "POLYTEXT_16";
   table8[OP_IMAGETEXT_8-offset] = "IMAGETEXT_8";
   table8[OP_IMAGETEXT_16-offset] = "IMAGETEXT_16";
   table8[OP_PUSH_PIXELS-offset] = "PUSH_PIXELS";
   table8[OP_FILL_SPANS-offset] = "FILL_SPANS";
   table8[OP_BEGIN_RENDERING-offset] = "BEGIN_RENDERING";
   table8[OP_END_RENDERING-offset] = "END_RENDERING";
   table8[OP_LOAD_RSL_TABLES-offset] = "LOAD_RSL_TABLES";
   table8[OP_BEGIN_PICK_M1-offset] = "BEGIN_PICK_M1";
   table8[OP_END_PICK_M1-offset] = "END_PICK_M1";
   table8[OP_BEGIN_ECHO-offset] = "BEGIN_ECHO";
   table8[OP_SET_ECHO_PARAMETERS-offset] = "SET_ECHO_PARAMETERS";
   table8[OP_END_ECHO-offset] = "END_ECHO";
   table8[OP_ADJUST_ELEMENT_COUNTER-offset] = "ADJUST_ELEMENT_COUNTER";
   table8[OP_INCREMENT_ELEMENT_COUNTER-offset] = "INCREMENT_ELEMENT_COUNTER";
   table8[OP_LOAD_PDRT_ATTRIBUTES-offset] = "LOAD_PDRT_ATTRIBUTES";
   table8[OP_LOAD_WORKSTATION_TRANSFORMATION-offset] = "LOAD_WORKSTATION_TRANSFORMATION";
   table8[OP_LOAD_FILTERS-offset] = "LOAD_FILTERS";
   table8[OP_SET_M1_CLIENT_CLIP_REGION-offset] = "SET_M1_CLIENT_CLIP_REGION";
   table8[OP_LOAD_VIEW_TABLE_DATA-offset] = "LOAD_VIEW_TABLE_DATA";
   table8[OP_CONDITIONAL_RETURN-offset] = "CONDITIONAL_RETURN";
   table8[OP_TEST_EXTENT_3-offset] = "TEST_EXTENT_3";
   table8[OP_TEST_EXTENT_2-offset] = "TEST_EXTENT_2";
   table8[OP_SET_CONDITION-offset] = "SET_CONDITION";
   table8[OP_SET_ATTRIBUTE_SOURCE_FLAG_W_CORR-offset] = "SET_ATTRIBUTE_SOURCE_FLAG_W_CORR";
   table8[OP_SET_TEXT_FONT_W_CORR-offset] = "SET_TEXT_FONT_W_CORR";
   table8[OP_SET_TEXT_INDEX_W_CORR-offset] = "SET_TEXT_INDEX_W_CORR";
   table8[OP_END_VIEW_W_CORR-offset] = "END_VIEW_W_CORR";
   table8[OP_SAVE_TSL-offset] = "SAVE_TSL";
   table8[OP_RELEASE_TSL-offset] = "RELEASE_TSL";
   table8[OP_BEGIN_VIEW-offset] = "BEGIN_VIEW";
   table8[OP_RESTORE_TSL_W_CORR-offset] = "RESTORE_TSL_W_CORR";
   table8[OP_SET_HLHSR_PROCESSING_OPTION-offset] = "SET_HLHSR_PROCESSING_OPTION";
   table8[OP_SURFACE_WITH_LINES_MODE-offset] = "SURFACE_WITH_LINES_MODE";
   table8[OP_SURFACE_POLYLINE_3-offset] = "SURFACE_POLYLINE_3";
   table8[OP_ANNOTATION_TEXT_GLYPHS-offset] = "ANNOTATION_TEXT_GLYPHS";
   table8[OP_ANNOTATION_TEXT_RELATIVE_GLYPHS-offset] = "ANNOTATION_TEXT_RELATIVE_GLYPHS";
   table8[OP_GEOMETRIC_TEXT_GLYPHS-offset] = "GEOMETRIC_TEXT_GLYPHS";
   table8[OP_CHARACTER_LINE_TEXT_GLYPHS-offset] = "CHARACTER_LINE_TEXT_GLYPHS";
   table8[OP_FAST_TEXT-offset] = "FAST_TEXT";
   table8[OP_FAST_TEXT_RELATIVE-offset] = "FAST_TEXT_RELATIVE";
   table8[OP_MATRIX_MODE-offset] = "MATRIX_MODE";
   table8[OP_PUSH_MODELLING_MATRIX_STACK-offset] = "PUSH_MODELLING_MATRIX_STACK";
   table8[OP_POP_MODELLING_MATRIX_STACK-offset] = "POP_MODELLING_MATRIX_STACK";
   table8[OP_SET_MODELLING_MATRIX_INVERSE_TRA-offset] = "SET_MODELLING_MATRIX_INVERSE_TRA";
   table8[OP_GET_CURRENT_CHARACTER_POSITION-offset] = "GET_CURRENT_CHARACTER_POSITION";
   table8[OP_GET_CURRENT_COLOR-offset] = "GET_CURRENT_COLOR";
   table8[OP_GET_CURRENT_MODELLING_MATRIX-offset] = "GET_CURRENT_MODELLING_MATRIX";
   table8[OP_GET_CURRENT_PROJECTION_MATRIX-offset] = "GET_CURRENT_PROJECTION_MATRIX";
   table8[OP_INITIALIZE_NAME_STACK-offset] = "INITIALIZE_NAME_STACK";
   table8[OP_LOAD_NAME-offset] = "LOAD_NAME";
   table8[OP_PUSH_NAME-offset] = "PUSH_NAME";
   table8[OP_POP_NAME-offset] = "POP_NAME";
   table8[OP_BEGIN_PICK_M1M-offset] = "BEGIN_PICK_M1M";
   table8[OP_END_PICK_M1M-offset] = "END_PICK_M1M";
   table8[OP_SET_3DM1M_COLOR_MODE-offset] = "SET_3DM1M_COLOR_MODE";
   table8[OP_SET_ANTI_ALIASED_LINE_MODE-offset] = "SET_ANTI_ALIASED_LINE_MODE";
   table8[OP_LOAD_GL_ATTRIBUTE_SETS-offset] = "LOAD_GL_ATTRIBUTE_SETS";
   table8[OP_SET_SCREEN_MASK-offset] = "SET_SCREEN_MASK";
   table8[OP_LMODEL_STATE-offset] = "LMODEL_STATE";
   table8[OP_LIGHT_LIST-offset] = "LIGHT_LIST";
   table8[OP_LIGHTING_MODEL-offset] = "LIGHTING_MODEL";
   table8[OP_DEFINE_LINE_STYLE-offset] = "DEFINE_LINE_STYLE";
   table8[OP_SET_LINE_STYLE_REPEAT_COUNT-offset] = "SET_LINE_STYLE_REPEAT_COUNT";
   table8[OP_C2H_PACKET-offset] = "C2H_PACKET";
   table8[OP_POLYGON_WITH_DATA_4-offset] = "POLYGON_WITH_DATA_4";
   table8[OP_TRIANGLE_MESH_WITH_DATA_4-offset] = "TRIANGLE_MESH_WITH_DATA_4";
   table8[OP_ELLIPTICAL_ARC_3_M1M-offset] = "ELLIPTICAL_ARC_3_M1M";
   table8[OP_ELLIPTICAL_ARC_3_FILLED-offset] = "ELLIPTICAL_ARC_3_FILLED";
   table8[OP_RASTER_TEXT-offset] = "RASTER_TEXT";
   table8[OP_DISJOINT_POLYLINE_M1M_2-offset] = "DISJOINT_POLYLINE_M1M_2";
   table8[OP_DISJOINT_POLYLINE_M1M_3-offset] = "DISJOINT_POLYLINE_M1M_3";
   table8[OP_BEGIN_LINE-offset] = "BEGIN_LINE";
   table8[OP_BEGIN_POINT-offset] = "BEGIN_POINT";
   table8[OP_BLAST_READ-offset] = "BLAST_READ";
   table8[OP_BLAST_WRITE-offset] = "BLAST_WRITE";
   table8[OP_BLAST_LOOP-offset] = "BLAST_LOOP";
   table8[OP_START_TRACE_FIFO-offset] = "START_TRACE_FIFO";
   table8[OP_STOP_TRACE_FIFO-offset] = "STOP_TRACE_FIFO";
   table8[OP_RESET_TRACE_FIFO-offset] = "RESET_TRACE_FIFO";
   table8[OP_START_DMA_TRACE_FIFO-offset] = "START_DMA_TRACE_FIFO";
   table8[OP_STOP_DMA_TRACE_FIFO-offset] = "STOP_DMA_TRACE_FIFO";
   table8[OP_CDD_CHAR_BLIT-offset] = "CDD_CHAR_BLIT";
   table8[OP_CDD_SET_COLORS-offset] = "CDD_SET_COLORS";
   table8[OP_CDD_CLEAR-offset] = "CDD_CLEAR";
   table8[OP_CDD_CRC-offset] = "CDD_CRC";


   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0x9nnn */
   /* *************************************************************** */

   offset = MIN_0x9nnn_OPCODE;
#ifdef debug
   printf ("Loading 0x9 table, offset: %x %d\n",offset,offset);
#endif

   table9[OP_SWITCH_CONTEXT-offset] = "SWITCH_CONTEXT";
   table9[OP_START_SOFT_RESET-offset] = "START_SOFT_RESET";
   table9[OP_COMPLETE_SOFT_RESET-offset] = "COMPLETE_SOFT_RESET";
   table9[OP_ECHO_COMMO-offset] = "ECHO_COMMO";


   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0xAnnn */
   /* *************************************************************** */

   offset = MIN_0xAnnn_OPCODE;
#ifdef debug
   printf ("Loading 0xA table, offset: %x %d\n",offset,offset);
#endif

   tableA[OP_SCREEN_BLANKING_CONTROL-offset] = "SCREEN_BLANKING_CONTROL";
   tableA[OP_SET_WINDOW_PARAMETERS_PCB-offset] = "SET_WINDOW_PARAMETERS_PCB";
   tableA[OP_ASSOC_WINDOW_WITH_COLOR_PALETTE-offset] = "ASSOC_WINDOW_WITH_COLOR_PALETTE";
   tableA[OP_REQUEST_TO_DMA_STRUCTURE_ELEMENT-offset] = "REQUEST_TO_DMA_STRUCTURE_ELEMENT";
   tableA[OP_CRTC_CONTROL-offset] = "CRTC_CONTROL";
   tableA[OP_DIAGNOSTICS-offset] = "DIAGNOSTICS";
   tableA[OP_CONTEXT_STATE_UPDATE_COMPLETE-offset] = "CONTEXT_STATE_UPDATE_COMPLETE";
   tableA[OP_CONTEXT_MEMORY_PINNED-offset] = "CONTEXT_MEMORY_PINNED";
   tableA[OP_CLEAR_CONTROL_PLANES-offset] = "CLEAR_CONTROL_PLANES";
   tableA[OP_DEFINE_CURSOR-offset] = "DEFINE_CURSOR";
   tableA[OP_SET_ACTIVE_CURSOR-offset] = "SET_ACTIVE_CURSOR";
   tableA[OP_MOVE_CURSOR-offset] = "MOVE_CURSOR";
   tableA[OP_HIDE_SHOW_ACTIVE_CURSOR-offset] = "HIDE_SHOW_ACTIVE_CURSOR";
   tableA[OP_BLINK_CURSOR_COLORS-offset] = "BLINK_CURSOR_COLORS";
   tableA[OP_FONT_PINNED-offset] = "FONT_PINNED";
   tableA[OP_FONT_MUST_BE_UNPINNED-offset] = "FONT_MUST_BE_UNPINNED";
   tableA[OP_FONT_REQUEST_RECEIVED-offset] = "FONT_REQUEST_RECEIVED";
   tableA[OP_LOAD_FRAME_BUFFER_COLOR_TABLE-offset] = "LOAD_FRAME_BUFFER_COLOR_TABLE";
   tableA[OP_BLINK_FRAME_BUFFER_COLOR_TABLE-offset] = "BLINK_FRAME_BUFFER_COLOR_TABLE";
   tableA[OP_LOAD_OVERLAY_PLANES_COLOR_TABLE-offset] = "LOAD_OVERLAY_PLANES_COLOR_TABLE";
   tableA[OP_BLINK_OVERLAY_PLANES_COLOR_TABLE-offset] = "BLINK_OVERLAY_PLANES_COLOR_TABLE";
   tableA[OP_MULTIMAP-offset] = "MULTIMAP";
   tableA[OP_ONEMAP-offset] = "ONEMAP";
   tableA[OP_SETMAP-offset] = "SETMAP";
   tableA[OP_CYCLE_COLOR_MAPS-offset] = "CYCLE_COLOR_MAPS";
   tableA[OP_START_TRACE_PCB-offset] = "START_TRACE_PCB";
   tableA[OP_STOP_TRACE_PCB-offset] = "STOP_TRACE_PCB";
   tableA[OP_RESET_TRACE_PCB-offset] = "RESET_TRACE_PCB";
   tableA[OP_START_DMA_TRACE_PCB-offset] = "START_DMA_TRACE_PCB";
   tableA[OP_STOP_DMA_TRACE_PCB-offset] = "STOP_DMA_TRACE_PCB";


   /* *************************************************************** */
   /* INTITIALIZE NAME TABLE FOR OPCODES THAT ARE OF THE FORM 0xFnnn */
   /* *************************************************************** */

   offset = MIN_0xFnnn_OPCODE;
#ifdef debug
   printf ("Loading 0xF table, offset: %x %d\n",offset,offset);
#endif

   tableF[OP_FIXED_TIME_DELAY-offset] = "FIXED_TIME_DELAY";
   tableF[OP_RANDOM_TIME_DELAY-offset] = "RANDOM_TIME_DELAY";
   tableF[OP_SET_TIMER-offset] = "SET_TIMER";
   tableF[OP_START_TIMER-offset] = "START_TIMER";
   tableF[OP_STOP_TIMER-offset] = "STOP_TIMER";
   tableF[OP_PRINT_TIMER-offset] = "PRINT_TIMER";
   tableF[OP_PRINT_TIME_AND_DATE-offset] = "PRINT_TIME_AND_DATE";
   tableF[OP_PRINT_COMMENT-offset] = "PRINT_COMMENT";
   tableF[OP_WAIT_KEY-offset] = "WAIT_KEY";
   tableF[OP_FETCH_MICROCODE_TRACE-offset] = "FETCH_MICROCODE_TRACE";
   tableF[OP_INVALID_DATA_IGNORED_BY_TED-offset] = "INVALID_DATA_IGNORED_BY_TED";
   tableF[OP_LOAD_DMA_BLIT-offset] = "LOAD_DMA_BLIT";
   tableF[OP_SAVE_DMA_BLIT-offset] = "SAVE_DMA_BLIT";
   tableF[OP_LOAD_DMA_SE-offset] = "LOAD_DMA_SE";
   tableF[OP_LOAD_PACKED_DMA_BLIT-offset] = "LOAD_PACKED_DMA_BLIT";
   tableF[OP_SAVE_PACKED_DMA_BLIT-offset] = "SAVE_PACKED_DMA_BLIT";
   tableF[OP_CLEAR_DMA_BLIT-offset] = "CLEAR_DMA_BLIT";
   tableF[OP_SAVE_DMA_TRACE_BUFFER-offset] = "SAVE_DMA_TRACE_BUFFER";
   tableF[OP_LOAD_DMA_FONT-offset] = "LOAD_DMA_FONT";
   tableF[OP_LOAD_CONTEXT_SE-offset] = "LOAD_CONTEXT_SE";
   tableF[OP_LOAD_CONTEXT_COMMAND_BLOCK-offset] = "LOAD_CONTEXT_COMMAND_BLOCK";
   tableF[OP_BEGIN_CONTEXT_SWITCH-offset] = "BEGIN_CONTEXT_SWITCH";
   tableF[OP_END_CONTEXT_SWITCH-offset] = "END_CONTEXT_SWITCH";
   tableF[OP_LOAD_CURSOR_IMAGE_COMMAND_BLOCK-offset] = "LOAD_CURSOR_IMAGE_COMMAND_BLOCK";
   tableF[OP_LOAD_COLOR_TABLE_COMMAND_BLOCK-offset] = "LOAD_COLOR_TABLE_COMMAND_BLOCK";
   tableF[OP_EXECUTE_TED_COMMAND-offset] = "EXECUTE_TED_COMMAND";
   tableF[OP_SET_SE_CORRELATOR_HW-offset] = "SET_SE_CORRELATOR_HW";
   tableF[OP_SAVE_PICK_BUFFER-offset] = "SAVE_PICK_BUFFER";
   tableF[OP_LOAD_IND_CONTROL_REGISTER-offset] = "LOAD_IND_CONTROL_REGISTER";
   tableF[OP_LOAD_IND_ADDRESS_REGISTER-offset] = "LOAD_IND_ADDRESS_REGISTER";
   tableF[OP_LOAD_IND_DATA_BUFFER-offset] = "LOAD_IND_DATA_BUFFER";
   tableF[OP_COMPUTE-offset] = "COMPUTE";
   tableF[OP_MODIFY-offset] = "MODIFY";
   tableF[OP_IF-offset] = "IF";
   tableF[OP_ELSE-offset] = "ELSE";
   tableF[OP_WHILE-offset] = "WHILE";
   tableF[OP_END-offset] = "END";
   tableF[OP_DO-offset] = "DO";
   tableF[OP_UNTIL-offset] = "UNTIL";
   tableF[OP_CURRENT_USER_ID_IS_OTHER-offset] = "CURRENT_USER_ID_IS_OTHER";
   tableF[OP_CURRENT_USER_ID_IS_TED-offset] = "CURRENT_USER_ID_IS_TED";
   tableF[OP_CURRENT_USER_ID_IS_CDD-offset] = "CURRENT_USER_ID_IS_CDD";
   tableF[OP_CURRENT_USER_ID_IS_DD-offset] = "CURRENT_USER_ID_IS_DD";
   tableF[OP_CURRENT_USER_ID_IS_3DM2-offset] = "CURRENT_USER_ID_IS_3DM2";
   tableF[OP_CURRENT_USER_ID_IS_3DM1-offset] = "CURRENT_USER_ID_IS_3DM1";
   tableF[OP_CURRENT_USER_ID_IS_X-offset] = "CURRENT_USER_ID_IS_X";
   tableF[OP_CURRENT_USER_ID_IS_RMS-offset] = "CURRENT_USER_ID_IS_RMS";
   tableF[OP_READ_IND_DATA_BUFFER-offset] = "READ_IND_DATA_BUFFER";
   tableF[OP_READ_ADAPTER_STATUS_CONTROL_BLK-offset] = "READ_ADAPTER_STATUS_CONTROL_BLK";
   tableF[OP_READ_FRAME_BUFFER_CTRL_STAT_BLK-offset] = "READ_FRAME_BUFFER_CTRL_STAT_BLK";
   tableF[OP_READ_CURRENT_CONTEXT_STATUS_BLK-offset] = "READ_CURRENT_CONTEXT_STATUS_BLK";
   tableF[OP_READ_CONTEXT_STATUS_BLOCK_X-offset] = "READ_CONTEXT_STATUS_BLOCK_X";
   tableF[OP_READ_CONTEXT_MEMORY_REQUEST_BLK-offset] = "READ_CONTEXT_MEMORY_REQUEST_BLK";
   tableF[OP_READ_FONT_REQUEST_BLOCK-offset] = "READ_FONT_REQUEST_BLOCK";
   tableF[OP_READ_CONTEXT_COMMAND_BLOCK-offset] = "READ_CONTEXT_COMMAND_BLOCK";
   tableF[OP_READ_COLOR_TABLE_COMMAND_BLOCK-offset] = "READ_COLOR_TABLE_COMMAND_BLOCK";
   tableF[OP_READ_CURSOR_IMAGE_COMMAND_BLOCK-offset] = "READ_CURSOR_IMAGE_COMMAND_BLOCK";
   tableF[OP_READ_CURRENT_3DM1_STATUS_BLOCK-offset] = "READ_CURRENT_3DM1_STATUS_BLOCK";
   tableF[OP_READ_3DM1_STATUS_BLOCK-offset] = "READ_3DM1_STATUS_BLOCK";
   tableF[OP_READ_CURRENT_3DM1M_STATUS_BLOCK-offset] = "READ_CURRENT_3DM1M_STATUS_BLOCK";
   tableF[OP_READ_3DM1M_STATUS_BLOCK-offset] = "READ_3DM1M_STATUS_BLOCK";
   tableF[OP_WRITE_IND_DATA_BUFFER-offset] = "WRITE_IND_DATA_BUFFER";
   tableF[OP_WRITE_ADAPTER_STATUS_CONTROL_BLK-offset] = "WRITE_ADAPTER_STATUS_CONTROL_BLK";
   tableF[OP_WRITE_FRAME_BUFFER_CTRL_STAT_BLK-offset] = "WRITE_FRAME_BUFFER_CTRL_STAT_BLK";
   tableF[OP_WRITE_CURRENT_CONTEXT_STATUS_BLK-offset] = "WRITE_CURRENT_CONTEXT_STATUS_BLK";
   tableF[OP_WRITE_CONTEXT_STATUS_BLOCK_X-offset] = "WRITE_CONTEXT_STATUS_BLOCK_X";
   tableF[OP_WRITE_CONTEXT_MEMORY_REQUEST_BLK-offset] = "WRITE_CONTEXT_MEMORY_REQUEST_BLK";
   tableF[OP_WRITE_FONT_REQUEST_BLOCK-offset] = "WRITE_FONT_REQUEST_BLOCK";
   tableF[OP_WRITE_CONTEXT_COMMAND_BLOCK-offset] = "WRITE_CONTEXT_COMMAND_BLOCK";
   tableF[OP_WRITE_COLOR_TABLE_COMMAND_BLOCK-offset] = "WRITE_COLOR_TABLE_COMMAND_BLOCK";
   tableF[OP_WRITE_CURSOR_IMAGE_COMMAND_BLOCK-offset] = "WRITE_CURSOR_IMAGE_COMMAND_BLOCK";
   tableF[OP_WRITE_CURRENT_3DM1_STATUS_BLOCK-offset] = "WRITE_CURRENT_3DM1_STATUS_BLOCK";
   tableF[OP_WRITE_3DM1_STATUS_BLOCK-offset] = "WRITE_3DM1_STATUS_BLOCK";
   tableF[OP_WRITE_CURRENT_3DM1M_STATUS_BLOCK-offset] = "WRITE_CURRENT_3DM1M_STATUS_BLOCK";
   tableF[OP_WRITE_3DM1M_STATUS_BLOCK-offset] = "WRITE_3DM1M_STATUS_BLOCK";
   tableF[OP_HARDWARE_RESET-offset] = "HARDWARE_RESET";

}
