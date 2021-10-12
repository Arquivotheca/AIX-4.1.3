static char sccsid[] = "@(#)60	1.1  src/bos/diag/tu/gga/param.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:38";
/*
*   COMPONENT_NAME: tu_gla
*
*   FUNCTIONS: none
*
*   ORIGINS: 27
*
*   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*   combined with the aggregated modules for this product)
*                    SOURCE MATERIALS
*
*   (C) COPYRIGHT International Business Machines Corp. 1995
*   All Rights Reserved
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*/

#include "ggamisc.h"

int ComputeParams(Screen, Input, Output, Rect, Params, Index, Switches)

ScrnSpec *Screen;               /* structure defining screen format */
SrcSpec *Input;         /* Defines the input formats for the streams     */
DstSpec *Output;                /* Defines the original output windows for the   */
                                /* streams                                       */
SegSpec *Rect;          /* Parameters for the rectangle.                 */
unsigned int *Params;           /* Pointer to array where computed parameters    */
                                /* are stored.                                   */
int Index;                      /* Initial pointer into the Params array.        */
int Switches;                   /* Switch options, mostly for hardware debug.    */

{
long hpi0, hpi1;        /* horizontal position increment */
int hfs0, hfs1; /* horizontal filter select      */
int ihp0, ihp1; /* initial horizontal position   */
long vpi0, vpi1;        /* vertical position increment   */
int vfs0, vfs1; /* vertical filter select        */
int ivp0, ivp1; /* initial vertical position     */
int v_context_0;        /* number of additional times    */
                        /* to read the first input line  */
                        /* to account for filter context */
int v_context_1;        /* number of additional times    */
                        /* to read the first input line  */
                        /* of the chrominance components */
                        /* to account for filter context */
int hpi_scale;
int chroma_h_scale;
int chroma_h_shift;
int ChromaWidth;
int ChromaxStartOffset;
int addr_shift;
int addr_mul;
int vpi_scale;
int chroma_v_scale;
int chroma_v_shift;
int v_start_context_0;
int v_end_context_0;
int ChromaHeight;
int ChromayStartOffset;
int align_mask_0;
int align_mask_1;
int out_width;
int out_height;
int first_pixel_0;
int first_pixel_1;
int last_pixel_0;
int last_pixel_1;
int x_start_pos_0;
int x_start_pos_1;
int x_end_pos_0;
int x_end_pos_1;
int y_start_pos_0;
int y_start_pos_1;
int y_end_pos_0;
int y_end_pos_1;
int first_line_0;
int first_line_1;
int last_line_0;
int last_line_1;
int v_addr_offset_0, v_addr_offset_1;
int h_off_0, h_off_1;
int isa_0_offset, isa_1_offset, isa_2_offset;
int in_width_0, in_width_1;
int in_height_0, in_height_1;
int osa;
int flip;

/*************************************************************************
 *                    Is there any work to do?                           *
 *************************************************************************/

if(Rect->Status == INVALID)     /* If rectangle structure invalid */
        return(Index);

/*************************************************************************
 *                  Compute some useful parameters                       *
 *************************************************************************/

/* compute some source format dependant parameters */
switch(Input->SrcMode)
    {
    case 0:     /* 32-bit packed RGB source */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 0;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x0;
            align_mask_1 = ~0x0;
            break;
    case 1:     /* 24-bit packed RGB source */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 2;
            addr_mul = 3;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    case 2:     /* Packed YUV 4:2:2 (YYUV) */
            hpi_scale = 12;
            chroma_h_scale = 2;
            chroma_h_shift = 1;
            ChromaWidth = Input->SrcWidth >> 1;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x0;
            break;
    case 3:     /* Packed YUV 4:2:2 (YUYV) */
            hpi_scale = 12;
            chroma_h_scale = 2;
            chroma_h_shift = 1;
            ChromaWidth = Input->SrcWidth >> 1;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x0;
            break;
    case 4:     /* Packed 16-bit RGB 5:5:5, each component will be converted */
                                /* to 8-bits by adding zeroes in low order bits */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x1;
            break;
    case 5: /* Packed 16-bit RGB 5:5:5, each component will be converted */
                                /* to 8-bits by copy most significant bits in low order bits */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x1;
            break;
    case 6: /* Packed 16-bit RGB 5:6:5, each component will be converted */
                                /* to 8-bits by adding zeroes in low order bits */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x1;
            break;
    case 7:  /* Packed 16-bit RGB 5:6:5, each component will be converted */
                                /* to 8-bits by copy most significant bits in low order bits */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 1;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x1;
            align_mask_1 = ~0x1;
            break;
    case 8: /* Planar YUV 4:4:4 */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 0;
            v_end_context_0 = 1;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    case 9: /* Planar YUV 4:2:2 */
            hpi_scale = 12;
            chroma_h_scale = 2;
            chroma_h_shift = 1;
            ChromaWidth = Input->SrcWidth >> 1;
            ChromaxStartOffset = 0;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 1;
            v_end_context_0 = 2;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    case 10:  /* Planar YUV 4:1:1 */
            hpi_scale = 11;
            chroma_h_scale = 4;
            chroma_h_shift = 2;
            ChromaWidth = Input->SrcWidth >> 2;
            ChromaxStartOffset = 0;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 1;
            v_end_context_0 = 2;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    case 13:  /* Planar YUV 4:2:0 */
            hpi_scale = 12;
            chroma_h_scale = 2;
            chroma_h_shift = 1;
            ChromaWidth = Input->SrcWidth >> 1;
            ChromaxStartOffset = 1024;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 12;
            chroma_v_scale = 2;
            chroma_v_shift = 1;
            v_start_context_0 = 1;
            v_end_context_0 = 2;
            ChromaHeight = Input->SrcHeight >> 1;
            ChromayStartOffset = 1024;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    case 14:  /* Planar YUV 4:1:0 */
            hpi_scale = 11;
            chroma_h_scale = 4;
            chroma_h_shift = 2;
            ChromaWidth = Input->SrcWidth >> 2;
            ChromaxStartOffset = 1536;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 11;
            chroma_v_scale = 4;
            chroma_v_shift = 2;
            v_start_context_0 = 1;
            v_end_context_0 = 2;
            ChromaHeight = Input->SrcHeight >> 2;
            ChromayStartOffset = 1536;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    default: /* Single component 8-bit greyscale */
            hpi_scale = 13;
            chroma_h_scale = 1;
            chroma_h_shift = 0;
            ChromaWidth = Input->SrcWidth;
            ChromaxStartOffset = 0;
            addr_shift = 2;
            addr_mul = 1;
            vpi_scale = 13;
            chroma_v_scale = 1;
            chroma_v_shift = 0;
            v_start_context_0 = 1;
            v_end_context_0 = 2;
            ChromaHeight = Input->SrcHeight;
            ChromayStartOffset = 0;
            align_mask_0 = ~0x3;
            align_mask_1 = ~0x3;
            break;
    }

/*************************************************************************
 *        Compute Horizontal Scaling Factors and Filter Select           *
 * The Horizontal Position Increment (hpi) is a fixed point number with  *
 * 12 fractional bits.                                                                                                                                   *
 * hpi defines the horizontal distance, in pixels, that the x position   *
 * within the source is incremented for each output increment of 1.      *
 * An hpi value of 4096 indicates a scale factor of 1. Values below 4096 *
 * indicate horizontal "stretching", values above 4096 indicate          *
 * horizontal "shrinking". For YUV modes where the chrominance is        *
 * horizontally subsampled(4:2:2, 4:1:1, 4:2:0, 4:1:0), the hpi for the  *
 * chrominance (hpi1) is derived within VideoPower by right shifting the *
 * hpi of the luminance (hpi0) by the appropriate amount (1 for          *
 * 4:2:2/4:2:0, 2 for 4:1:1/4:1:0). To ensure correct registration of    *
 * the scaled luminance and chrominance components, hpi0 must be rounded *
 * such that for modes where hpi1 is derived by shifting hpi0, the       *
 * appropriate number of low order bits are 0. In YUV 4:2:2 and 4:2:0    *
 * source modes, bit[0] of hpi0 must be 0. In YUV 4:1:1 and 4:1:0 source *
 * modes, bits[1:0] of hpi0 must be 0. Ignoring rounding requirements,   *
 * hpi0 is computed as follows:-                                                                                                                 *
 * hpi0 = (source_width * 4096)/destination_width.                                                               *
 *************************************************************************/

hpi1 = (((Input->SrcWidth << hpi_scale)/Output->DstDx) + 1) >> 1;
hpi0 = hpi1 << chroma_h_shift;

if(Switches & 0x2)              /* disable the filters */
        {
        hfs0 = 0;
        hfs1 = 0;
        }
else
        {
        if(hpi0 < 4096)         /* interpolation, use 4-tap */
        hfs0 = 2;
        else if (hpi0 < 8192)   /* < 2:1 decimation, use 2-tap average */
        hfs0 = 1;
        else                    /* > 2:1 decimation, use 4-tap average */
        hfs0 = 3;

        if(hpi1 < 4096)         /* interpolation */
        hfs1 = 2;
        else if (hpi1 < 8192)   /* < 2:1 decimation */
        hfs1 = 1;
        else                    /* > 2:1 decimation */
        hfs1 = 3;
        }

/*************************************************************************
 *         Compute Vertical Scaling Factors and Filter Select            *
 * The Vertical Position Increment (vpi) isa  fixed point number with 12 *
 * fractional bits.                                                                                                                                              *
 * vpi defines the horizontal distance, in lines, that the y position    *
 * within the source is incremented for each output line increment of 1. *
 * An vpi value of 4096 indicates a scale factor of 1. Values below 4096 *
 * indicate vertical "stretching", values above 4096 indicate            *
 * vertical "shrinking". For YUV modes where the chrominance is          *
 * vertically subsampled (4:2:0, 4:1:0), the vpi for the  chrominance    *
 * (vpi1) is derived within VideoPower by right shifting the vpi of the  *
 * luminance (vpi0) by the appropriate amount (1 for 4:2:0, 2 for 4:1:0).*
 * To ensure correct registration of the scaled luminance and            *
 * chrominance components, vpi0 must be rounded such that for modes      *
 * where vpi1 is derived by shifting vpi0, the appropriate number of low *
 * order bits are 0. In YUV 4:2:0 source mode, bit[0] of vpi0 must be 0. *
 * In YUV 4:1:0 source  mode, bits[1:0] of vpi0 must be 0. Ignoring      *
 * rounding requirements, vpi0 is computed as follows:-                                          *
 * vpi0 = (source_height * 4096)/destination_height.                                                     *
 *************************************************************************/

vpi1 = (((Input->SrcHeight << vpi_scale)
                        /Output->DstDy) + 1) >> 1;
vpi0 = vpi1 << chroma_v_shift;

if(Switches & 0x2)
        {
        vfs0 = 4;
        vfs1 = 2;
        }
else
        {
        if(vpi0 < 4096)         /* interpolation */
        {
        if(Input->SrcMode > 8)
                vfs0 = 2;
        else
                vfs0 = 0;
        }
        else if (vpi0 < 8192)   /* < 2:1 decimation */
        vfs0 = 1;
        else                    /* > 2:1 decimation */
        {
        if(Input->SrcMode > 8)
                vfs0 = 3;
        else
        vfs0 = 1;
        }


        if(vpi1 < 4096)         /* interpolation */
        vfs1 = 0;
        else                    /* < 2:1 decimation */
        vfs1 = 1;
        }


/*************************************************************************
 *                     Compute the x start positions                     *
 * These are the start positions of the left edge of the rectangle       *
 * within the input frame, expressed as a fixed point number with twelve *
 * bits of fractional accuracy. They are given by:-                                                      *
 * X_start = (X_offset * hpi) - StartOffset                                                                              *
 * Start_Offset is the horizontal position in the source corresponding   *
 * to the left edge of the destination window (NOT the clipped rectangle *
 * For Packed RGB modes only x_start_pos_0 is actually used, and the     *
 * StartOffset is 0.                                                                                                                                             *
 * For Planar YUV modes x_start_pos_0 refers to the Y component, and the *
 * StartOffset is 0. x_start_pos_1 refers to the chrominance components, *
 * and the StartOffset depends on the chrominance sampling structure.    *
 * For YUV 4:2:2 and 4:1:1 where the chrominance samples are "cosited"   *
 * with the luminance samples, StartOffset is 0. For YUV 4:2:0 modes     *
 * where the chrominance samples are centered in each group of 4         *
 * luminance samples, the first chrominance sample is horizontally       *
 * offset from the first luminance sample by 1/4 of the chrominance      *
 * sample spacing. Expressed as a fixed point number with 12 fractional  *
 * bits, the ChromaxStartOffset is 1024.                                                                                         *
 * For YUV 4:1:0 modes where the chrominance samples are centered in     *
 * each group of 8 luminance samples, the first chrominance sample is    *
 * horizontally offset from the first luminance sample by 3/8 of the     *
 * chrominance sample spacing. Expressed as a fixed point number with 12 *
 * fractional bits, the ChromaxStartOffset is 1536.                                                      *
 *************************************************************************/

x_start_pos_0 = (Rect->XOff * hpi0);
x_start_pos_1 = (x_start_pos_0 >> chroma_h_shift) - ChromaxStartOffset;

/*************************************************************************
 *                      Compute the x end position                       *
 * This is the position within the input frame of the rightmost pixel    *
 * within the rectangle, expressed with 12 bits of fractional accuracy.  *
 * Given by:- end_position = start_position + ((rect_width - 1) * hpi)   *
 *************************************************************************/

out_width = Rect->X1 - Rect->X0; /* actually width - 1 */
x_end_pos_0 = x_start_pos_0 + (out_width * hpi0);
x_end_pos_1 = x_start_pos_1 + (out_width * hpi1);

/*************************************************************************
 *                     Compute the y start position                      *
 * These are the start positions of the top edge of the rectangle        *
 * within the input frame, expressed as a fixed point number with twelve *
 * bits of fractional accuracy. They are given by:-                                                      *
 * Y_start = (Y_offset * vpi) - yStartOffset                                                                             *
 * yStartOffset is the vertical position in the source corresponding     *
 * to the top edge of the destination window (NOT the clipped rectangle) *
 * For Packed RGB modes only y_start_pos_0 is actually used, and the     *
 * yStartOffset is 0.                                                                                                                                    *
 * For Planar YUV modes y_start_pos_0 refers to the Y component, and the *
 * yStartOffset is 0. y_start_pos_1 refers to the chrominance components,*
 * and the yStartOffset depends on the chrominance sampling structure.   *
 * For YUV 4:2:2 and 4:1:1 where the chrominance samples are "cosited"   *
 * with the luminance samples, yStartOffset is 0. For YUV 4:2:0 modes    *
 * where the chrominance samples are centered in each group of 4         *
 * luminance samples, the first chrominance sample is vertically         *
 * offset from the first luminance sample by 1/4 of the chrominance      *
 * sample spacing. Expressed as a fixed point number with 12 fractional  *
 * bits, the ChromayStartOffset is 1024.                                                                                         *
 * For YUV 4:1:0 modes where the chrominance samples are centered in     *
 * each group of 8 luminance samples, the first chrominance sample is    *
 * vertically offset from the first luminance sample by 3/8 of the       *
 * chrominance sample spacing. Expressed as a fixed point number with 12 *
 * fractional bits, the ChromayStartOffset is 1536.                                                      *
 *************************************************************************/

y_start_pos_0 = (Rect->YOff * vpi0);
y_start_pos_1 = (y_start_pos_0 >> chroma_v_shift) - ChromayStartOffset;

/*************************************************************************
 *                     Compute the y end position                        *
 * This is the position within the input frame of the bottommost line    *
 * within the rectangle, expressed with 12 bits of fractional accuracy.  *
 * Given by:- end_position = start_position + ((rect_height - 1) * vpi)  *
 *************************************************************************/

out_height = Rect->Y1 - Rect->Y0; /* actually height - 1 */
y_end_pos_0 = y_start_pos_0 + (out_height * vpi0);
y_end_pos_1 = y_start_pos_1 + (out_height * vpi1);

/*************************************************************************
 *                  Compute the first and last pixels                    *
 * first_pixel is the x coordinate within the input frame of the first   *
 * pixel required to generate the output rectangle. The "integer" part   *
 * of x_start_pos (x_start_pos >> 12) gives the nearest pixel to the     *
 * left of the position within the input frame of the left edge of the   *
 * rectangle. Since the horizontal filter has four taps, an additional   *
 * pixel to the left is required, so one is subtracted. Since VideoPower *
 * can only read source frame lines starting at DWORD boundaries         *
 * (3 DWORD boundaries for packed 24-bit RGB source). The first_pixel    *
 * parameter is rounded down to the nearest appropriate boundary.        *
 * If first_pixel is negative, it is clipped to 0.                                                               *
 * last_pixel is the x coordinate within the input frame of the last     *
 * pixel required to generate the output rectangle. The "integer" part   *
 * of x_end_pos (x_end_pos >> 12) gives the nearest pixel to the         *
 * left of the position within the input frame of the right edge of the  *
 * rectangle. Since the horizontal filter has four taps, an additional   *
 * two pixels to the right are required, so 2 is added.                                          *
 * If last_pixel is greater than the width of the source image, it is    *
 * clipped to source_width - 1 (coordinate of the last pixel in a source *
 * line).                                                                                                                                                                        *
 * first_pixel and last_pixel are computed indepenently for the luminance *
 * and chrominance components. However, in packed YUV source modes, the  *
 * values may need adjusting to ensure that first_pixel_0 and            *
 * first_pixel_1 are in the same DWORD. Similarly in this mode,          *
 * last_pixel_0 and last_pixel_1 must be in the same DWORD.                                      *
 *************************************************************************/

first_pixel_0 = ((x_start_pos_0 >> 12) - 1) & align_mask_0;
first_pixel_1 = ((x_start_pos_1 >> 12) - 1) & align_mask_1;
last_pixel_0 = (x_end_pos_0 >> 12) + 2;
last_pixel_1 = (x_end_pos_1 >> 12) + 2;
if(PACKYUV)
        {
   first_pixel_0 = min(first_pixel_0, (first_pixel_1 << 1));
   first_pixel_1 = first_pixel_0 >> 1;
   last_pixel_0 = max(last_pixel_0, (last_pixel_1 << 1));
   }

if(first_pixel_0 < 0)
    {
    first_pixel_0 = 0;
    }
if(first_pixel_1 < 0)
    {
    first_pixel_1 = 0;
    }
if(last_pixel_0 > (Input->SrcWidth - 1))
    {
    last_pixel_0 = Input->SrcWidth - 1;
    }
if(last_pixel_1 > (ChromaWidth - 1))
    {
    last_pixel_1 = ChromaWidth - 1;
    }

/*************************************************************************
 *              Compute the first and last lines read in                 *
 * first_line is the y coordinate within the input frame of the first    *
 * line required to generate the output rectangle. The "integer" part    *
 * of y_start_pos (y_start_pos >> 12) gives the nearest pixel to the     *
 * top of the position within the input frame of the top edge of the     *
 * rectangle. Since the vertical filter has either two or four taps,     *
 * depending on the source format and the component, an additional       *
 * line above may be required, (defined by v_start_context).             *
 * last_line is the y coordinate within the input frame of the last      *
 * line required to generate the output rectangle. The "integer" part    *
 * of y_end_pos (y_end_pos >> 12) gives the nearest line to the             *
 * top of the position within the input frame of the top edge of the     *
 * rectangle. Since the vertical filter has two or four taps, an         *
 * additional one or two lines below are required, (defined by           *
 * v_end_context)                                                                                                                                                        *
 * If last_line is greater than the height of the source image, it is    *
 * clipped to source_height - 1 (coordinate of the last line in a source *
 * frame).                                                                                                                                                                       *
 *************************************************************************/

first_line_0 = (y_start_pos_0 >> 12) - v_start_context_0;
first_line_1 = (y_start_pos_1 >> 12);
last_line_0 = (y_end_pos_0 >> 12) + v_end_context_0;
last_line_1 = (y_end_pos_1 >> 12) + 1;

if(last_line_0 > (Input->SrcHeight - 1))
    {
    last_line_0 = Input->SrcHeight - 1;
    }
if(last_line_1 > (ChromaHeight - 1))
    {
    last_line_1 = ChromaHeight - 1;
    }

/*************************************************************************
 *                  Compute the initial x position                                   *
 * To reduce unecessary source reads, VideoPower will only read lines    *
 * starting at x coordinate "first_pixel". x_start_pos is the x start    *
 * position with respect to the left edge of the input frame. ihp is the *
 * position with respect to the section of the line that is actually     *
 * read in to VideoPower. ihp (initial horizontal position) is computed  *
 * by subtracting first_pixel from the "integer" part of x_start_pos.    *
 *************************************************************************/

ihp0 = x_start_pos_0 - (first_pixel_0 << 12);
ihp1 = x_start_pos_1 - (first_pixel_1 << 12);

/*************************************************************************
 *                   Compute the initial y position                      *
 * To reduce unecessary source reads, VideoPower will only read lines    *
 * starting at y coordinate "first_line". y_start_pos is the y start     *
 * position with respect to the top edge of the input frame. ivp is the  *
 * position with respect to the first line that is actually read in to   *
 * VideoPower. ivp (initial vertical position) is computed by subtracting*
 * first_line from the "integer" part of y_start_pos. If first line is   *
 * negative, VideoPower must read the first line a number of times to    *
 * provide the context for the vertical filter. v_context defines the    *
 * number of additional times that VideoPower must read the first line   *
 *************************************************************************/

if(first_line_0 < 0)
    {
    ivp0 = y_start_pos_0;
    v_context_0 = -first_line_0;
    first_line_0 = 0;
    }
else
    {
    ivp0 = y_start_pos_0 - (first_line_0 << 12);
    v_context_0 = 0;
    }

if(first_line_1 < 0)
    {
    ivp1 = y_start_pos_1;
    v_context_1 = -first_line_1;
    first_line_1 = 0;
    }
else
    {
    ivp1 = y_start_pos_1 - (first_line_1 << 12);
    v_context_1 = 0;
    }

/*************************************************************************
 *      Compute the vertical address offsets within source image         *
 * v_addr_offset is an offset into a source frame which points to the    *
 * start of the first line which will be read by VideoPower. For Planar  *
 * YUV source frames, there are two offsets computed, one for the Y      *
 * component and one for the chrominance components. The offset depends  *
 * on the orientation of the image. VideoPower always draws from screen  *
 * top to screen bottom. If the origin of the source is at the top left  *
 * corner, the image is read top to bottom and the offset is given by    *
 * offset(DWORDS) = first_line * stride(DWORDS)                                                                  *
 * If the origin of the source is at the bottom left (the bottom line    *
 * occupies the lowest address in offscreen memory), the offset must     *
 * point to the line (in_height - 1) - first_line, and the offset is     *
 * given by:-                                                                                                                                                            *
 * offset(DWORDS) = (in_height - first_line - 1) * stride(DWORDS)        *
 * note that in this case, the stride programmed into VideoPower will be *
 * negative.                                                                                                                                                             *
 *************************************************************************/

flip = (Input->SrcOrigin ^ (Switches >> 3)) & 0x1;
if(flip == 1)
        {
        v_addr_offset_0 = first_line_0 * Input->SrcStride;
        v_addr_offset_1 = (first_line_1 * Input->SrcStride)
                                                 >> chroma_h_shift;
        }
else
        {
        v_addr_offset_0 = (Input->SrcHeight - first_line_0 - 1)
                                                  * Input->SrcStride;
        v_addr_offset_1 = (Input->SrcChrHeight - first_line_1 - 1)
                                                  * Input->SrcChrStride;
        }


/*************************************************************************
 *               Compute the horizontal address offsets                  *
 * h_off is the address offset in DWORDS from the start of the line to   *
 * first_pixel.                                                                                                                                                  *
 *************************************************************************/

h_off_0 = (first_pixel_0 * addr_mul) >> addr_shift;
h_off_1 = (first_pixel_1 * addr_mul) >> addr_shift;

/*************************************************************************
 *                Compute the combined address offsets                   *
 * this is the offset from the start of the image to first_pixel on      *
 * first_line, for each component.                                                                                                       *
 *************************************************************************/

isa_0_offset = v_addr_offset_0 + h_off_0;
isa_1_offset = v_addr_offset_1 + h_off_1;
isa_2_offset = v_addr_offset_1 + h_off_1;

/*************************************************************************
 *                       Compute the input width                         *
 * This is actually (input_width - 1) and is the value programmed into   *
 * VideoPower. For Planar YUV formats, there is one value for the Y      *
 * component, and one value for the chrominance components.                                      *
 *************************************************************************/

in_width_0 = last_pixel_0 - first_pixel_0;
in_width_1 = last_pixel_1 - first_pixel_1;

/*************************************************************************
 *                      Compute the input height                         *
 * This is actually (input_height - 1) and is the value programmed into  *
 * VideoPower. For Planar YUV formats, there is one value for the Y      *
 * component, and one value for the chrominance components.                                      *
 *************************************************************************/

in_height_0 = last_line_0 - first_line_0;
in_height_1 = last_line_1 - first_line_1;

/*************************************************************************
 *                  Compute the output start address                     *
 * This is the byte address for the first output pixel                                           *
 *************************************************************************/

osa = (Rect->Y0 * Screen->ScrnPitch * 4)
    + (Rect->X0 * Screen->ScrnBytesPerPel);

/************************************************************************
 *                                                                      *
 *                    Assemble VideoPower registers                     *
 *                                                                      *
 ************************************************************************/


/* parameter 1 - isa0 */
*Params++ = 0x8000000 | ((Input->SrcAddr0 + isa_0_offset) & 0xfffff);
Index++;

/* if planar, need parameters 2 and 3 - isa1 and isa2 */
if(Input->SrcComps > 1)
        {
        *Params++ = 0x10000000 | ((Input->SrcAddr1 + isa_1_offset) & 0xfffff);
        Index++;
        *Params++ = 0x18000000 | ((Input->SrcAddr2 + isa_2_offset) & 0xfffff);
        Index++;
        }

/* parameter 4 - id0 */
*Params++ = 0x20000000 | ((in_width_0 & 0x1ff) << 16) | (in_height_0 & 0x3ff);
Index++;

/* if planar, need parameter 5 - id1 */
if((Input->SrcComps > 1) || PACKYUV)
        {
        *Params++ = 0x28000000 | ((in_width_1 & 0x1ff) << 16) | (in_height_1 & 0x3ff);
        Index++;
        }

/* parameter 6 - inp */
if(flip == 1) /* source origin is top left */
        *Params = 0x30000000 | (Input->SrcMode << 16) | Input->SrcStride;
else          /* source origin is bottom left, make pitch negative */
        *Params = 0x30000000 | (Input->SrcMode << 16) | (-Input->SrcStride & 0x1fff);
if(Switches & 0x10)
        {
        *Params++ |= 0x1000000;
        Index++;
        }
else
        {
        *Params++ |= 0;
        Index++;
        }

/* parameter 7  - od */
*Params++ = 0x38000000 | (out_width << 16) | out_height;
Index++;

/* parameter 8 - trg */
*Params = 0x40000000 | ((Rect->Y0 + 100) << 12) | Rect->Y0;
if(Switches & 0x80000000)
        {
        *Params++ |= 0x3000000;
        Index++;
        }
else
        {
        *Params++ |= 0;
        Index++;
        }

/* parameter 9 - ivp0 */
*Params++ = 0x48000000 | (vfs0 << 20) | (v_context_0 << 16) | (ivp0 & 0xffff);
Index++;

/* parameter 10 - ivp1 */
*Params++ = 0x50000000 | (vfs1 << 20) | (v_context_1 << 16) | (ivp1 & 0xffff);
Index++;

/* parameter 11 - vpi */
*Params++ = 0x58000000 | (vpi0 & 0xffff);
Index++;

/* parameter 12 - ihp0 */
*Params++ = 0x60000000 | (hfs0 << 20) | (ihp0 & 0xffff);
Index++;

/* parameter 13 - ihp1 */
*Params++ = 0x68000000 | (hfs1 << 20) | (ihp1 & 0xffff);
Index++;

/* parameter 14 - hpi */
*Params++ = 0x70000000 | (hpi0 & 0xffff);
Index++;

/* parameter 15 - cspd */
if(Switches & 0x4)      /* Disable Dithering */
        *Params = 0x78000000;
else
        *Params = 0x78000020;

if(Switches & 0x80)
        *Params |= 0x10;

if(YUVSRC && !(Switches & 0x01))
        {
        *Params++ |= 0x4;
        Index++;
        }
else
        {
        *Params++ |= 0x0;
        Index++;
        }

/* parameter 16 - osa */
*Params++ = 0x80000000 | (osa & 0x3fffff);
Index++;

return(Index);

}
