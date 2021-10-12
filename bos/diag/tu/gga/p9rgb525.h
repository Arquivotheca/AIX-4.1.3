/* @(#)81	1.1  src/bos/diag/tu/gga/p9rgb525.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:15 */
/*
*
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
#define RGB525_MISC_CLK     0x0002
#define RGB525_SYNC_CTL     0x0003
#define RGB525_SYNC_POS     0x0004
#define RGB525_PWR_MGT      0x0005
#define RGB525_DAC_OP       0x0006
#define RGB525_PALETTE_CTL  0x0007
#define RGB525_PIXEL_FMT    0x000a
#define RGB525_8BPP_CTL     0x000b
#define RGB525_16BPP_CTL    0x000c
#define RGB525_24BPP_CTL    0x000d
#define RGB525_32BPP_CTL    0x000e
#define RGB525_PLL_CTL1     0x0010
#define RGB525_PLL_CTL2     0x0011
#define RGB525_PLL_DIV      0x0014
#define RGB525_F0           0x0020
#define RGB525_F1           0x0021
#define RGB525_F2           0x0022
#define RGB525_F3           0x0023
#define RGB525_F4           0x0024
#define RGB525_F5           0x0025
#define RGB525_F6           0x0026
#define RGB525_F7           0x0027
#define RGB525_F8           0x0028
#define RGB525_F9           0x0029
#define RGB525_F10          0x002a
#define RGB525_F11          0x002b
#define RGB525_F12          0x002c
#define RGB525_F13          0x002d
#define RGB525_F14          0x002e
#define RGB525_F15          0x002f
#define RGB525_CSR_CTL      0x0030
#define RGB525_CSR_X_LOW    0x0031
#define RGB525_CSR_X_HIGH   0x0032
#define RGB525_CSR_Y_LOW    0x0033
#define RGB525_CSR_Y_HIGH   0x0034
#define RGB525_CSR_X_HOT    0x0035
#define RGB525_CSR_Y_HOT    0x0036
#define RGB525_CSR_1_RED    0x0040
#define RGB525_CSR_1_GREEN  0x0041
#define RGB525_CSR_1_BLUE   0x0042
#define RGB525_CSR_2_RED    0x0043
#define RGB525_CSR_2_GREEN  0x0044
#define RGB525_CSR_2_BLUE   0x0045
#define RGB525_CSR_3_RED    0x0046

#define RGB525_CSR_3_GREEN  0x0047
#define RGB525_CSR_3_BLUE   0x0048
#define RGB525_BORDER_RED   0x0060
#define RGB525_BORDER_GREEN 0x0061
#define RGB525_BORDER_BLUE  0x0062
#define RGB525_MISC_CTL1    0x0070
#define RGB525_MISC_CTL2    0x0071
#define RGB525_MISC_CLT3    0x0072
#define RGB525_DAC_SENSE    0x0082
#define RGB525_MISR_RED     0x0084
#define RGB525_MISR_GREEN   0x0086
#define RGB525_MISR_BLUE    0x0088
#define RGB525_PLL_VCO      0x008e
#define RGB525_PLL_REF      0x008f
#define RGB525_VRAM_MASKH   0x0090
#define RGB525_VRAM_MASKL   0x0091
#define RGB525_CURSOR_ARRAY 0x0100

static struct {
      ushort addr;
      uchar  data;                       /* Reg                                  */
      } rgb525_init_tab[] = {           /* Num  - Function used                 */

        RGB525_MISC_CLK    , 0x27,      /* 0x02 - SLCK,DCLK,Disable PLL         */
        RGB525_SYNC_CTL    , 0x80,      /* 0x03 - Do not add delay.                                */
        RGB525_SYNC_POS    , 0x00,      /* 0x04                                 */
        RGB525_PWR_MGT     , 0x00,      /* 0x05 - Full up power -> HOG Mode     */
        RGB525_DAC_OP      , 0x02,      /* 0x06 - Set Fast Slew Rate            */
        RGB525_PALETTE_CTL , 0x00,      /* 0x07                                 */
        RGB525_PIXEL_FMT   , 0x03,      /* 0x0a - Set 8BPP Mode                 */
        RGB525_8BPP_CTL    , 0x00,      /* 0x0b - Normal Palette Mode           */
        RGB525_16BPP_CTL   , 0x00,      /* 0x0c - Normal 16bpp mode             */
        RGB525_24BPP_CTL   , 0x01,      /* 0x0d - Direct Color                  */
        RGB525_32BPP_CTL   , 0x00,      /* 0x0e - Direct Color                  */
        RGB525_PLL_CTL1    , 0x02,      /* 0x10 - Use REFCLK, and Internal FS   */
        RGB525_PLL_CTL2    , 0x00,      /* 0x11 - Set same as S3 CR42[3:0]      */
        RGB525_PLL_DIV     , (50/2),    /* 0x14 - Set Reference Divider MHZ/2   */

                /* Even though the 16 clock values are loaded during init,      */
                /* entry F0 is the only one used.  The ecrtc table has the      */
                /* exact clock value to load into the F0 register. Ctl1 and     */
                /* ctl2 will always point to internal FS:0.                     */

        RGB525_F0          , 0x24,      /* 0x20 - 25.175 (act 25.25)            */
        RGB525_F1          , 0x30,      /* 0x21 - 28.332 (act 28.25)            */
        RGB525_F2          , 0x4f,      /* 0x22 - 40.00                         */
        RGB525_F3          , 0xcf,      /* 0x23  160.00                         */
        RGB525_F4          , 0x63,      /* 0x24   50.00                         */
        RGB525_F5          , 0x8c,      /* 0x25   77.00                         */
        RGB525_F6          , 0x47,      /* 0x26   36.00                         */
        RGB525_F7          , 0x59,      /* 0x27   44.889 (act 45.00)            */
        RGB525_F8          , 0xc0,      /* 0x28  130.00                         */
        RGB525_F9          , 0xb7,      /* 0x29  120.00                         */
        RGB525_F10         , 0x8f,      /* 0x2a   80.00                         */
        RGB525_F11         , 0x3d,      /* 0x2b   31.50                         */
        RGB525_F12         , 0xad,      /* 0x2c  110.00                         */
        RGB525_F13         , 0x80,      /* 0x2d   65.00                         */
        RGB525_F14         , 0x8a,      /* 0x2e   75.00                         */
        RGB525_F15         , 0x9e,      /* 0x2f   94.50  (act 95.00)            */
        RGB525_CSR_CTL     , 0x00,      /* 0x30                                 */
        RGB525_CSR_X_LOW   , 0x00,      /* 0x31                                 */
        RGB525_CSR_X_HIGH  , 0x00,      /* 0x32                                 */
        RGB525_CSR_Y_LOW   , 0x00,      /* 0x33                                 */
        RGB525_CSR_Y_HIGH  , 0x00,      /* 0x34                                 */
        RGB525_CSR_X_HOT   , 0x00,      /* 0x35                                 */
        RGB525_CSR_Y_HOT   , 0x00,      /* 0x36                                 */
        RGB525_CSR_1_RED   , 0x00,      /* 0x40                                 */
        RGB525_CSR_1_GREEN , 0x00,      /* 0x41                                 */
        RGB525_CSR_1_BLUE  , 0x00,      /* 0x42                                 */
        RGB525_CSR_2_RED   , 0x00,      /* 0x43                                 */
        RGB525_CSR_2_GREEN , 0x00,      /* 0x44                                 */
        RGB525_CSR_2_BLUE  , 0x00,      /* 0x45                                 */
        RGB525_CSR_3_RED   , 0x00,      /* 0x46                                 */
        RGB525_CSR_3_GREEN , 0x00,      /* 0x47                                 */
        RGB525_CSR_3_BLUE  , 0x00,      /* 0x48                                 */
        RGB525_BORDER_RED  , 0x00,      /* 0x60                                 */
        RGB525_BORDER_GREEN, 0x00,      /* 0x61                                 */
        RGB525_BORDER_BLUE , 0x00,      /* 0x62                                 */
        RGB525_MISC_CTL1   , 0x01,      /* 0x70 - Select 64 bit SD bus, PLL     */
        RGB525_MISC_CTL2   , 0x45,      /* 0x71 - Select 8 bit color, SID port  */
        RGB525_MISC_CLT3   , 0x00,      /* 0x72                                 */
        RGB525_VRAM_MASKH  , 0x00,      /* 0x90                                 */
        RGB525_VRAM_MASKL  , 0x01,      /* 0x91                                 */

        0x00,0x00               /* Terminate the list                           */
                            };
#define RGB525_CSR_ARRAY      0x0100
