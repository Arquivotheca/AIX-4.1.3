
/*
** BAUD Hardware Registers
**
**
** (C) Copyright IBM Corp., 1994
** All rights reserved.
** U.S. Government Users Restricted Rights -- Use, duplication,
** or disclosure restricted by GSA ADP Schedule Contract with
** IBM Corp.
** Program Property of IBM.
**
*/


/* POS REGISTERS */
#define POS0    0

#define POS1    1

#define POS2    2               /* Program last */
#define LBARB   0x80
#define POS1026 0x40
#define SYN_BUR 0x20
#define PLAYARB 0x1E
#define ENABLE  0x01

#define POS3    3

#define POS4    4              /* 0x01 for IRQ10 */
#define INT4MAP 0xC0           /* 0x00 for IRQ09 */
#define INT3MAP 0x30           /* 0x02 for IRQ11 */
#define INT2MAP 0x0C           /* 0x03 for IRQ12 */
#define INT1MAP 0x03

#define POS5    5              /* 0x8X */
#define CHCK_N  0x80
#define OPTCHCK 0x40
#define FAIR    0x10
#define CAPTARB 0x0F

#define POS6    6

#define POS7    7


/*  MCI94C18A REGISTERS */

#define MCI_REG0        0x0
#define SW_RESET        0x80
#define EN_TMR          0x40
#define IO_WAIT         0x38
#define MEM_WAIT        0x07

#define MCI_REG1        0x1
#define EN_BURST2       0x20
#define EN_BURST1       0x10
#define EN_DMA2         0x02
#define EN_DMA1         0x01

#define MCI_INTR_EN     0x2
#define MCI_INTR_AK     0x3
#define MCI_INTR_ST     0x3
#define TMR_INTR        0x40
#define INT4            0x20
#define INT3            0x10
#define INT2            0x08
#define INT1            0x04

#define MCI_TIMER       0x4

#define MCI_REG5        0x5
#define ANDOR_DMA2      0x80
#define ANDOR_DMA1      0x40
#define SW_REQ2         0x02
#define SW_REQ1         0x01


/* Audio Control Registers
 * -----------------------                                                    */

#define MCI_BASE                  0
#define AUDIO_BASE                MCI_BASE + 0x08

#define AUDIO_INDEX_REG           AUDIO_BASE + 0x00
#define AUDIO_INDEXED_DATA_REG    AUDIO_BASE + 0x01
#define AUDIO_STATUS_REG          AUDIO_BASE + 0x02
#define AUDIO_PIO_DATA_REG        AUDIO_BASE + 0x03

#define L_ADC_INPUT_REG           0x00
#define R_ADC_INPUT_REG           0x01
#define LAUX1_INPUT_REG           0x02
#define RAUX1_INPUT_REG           0x03
#define LAUX2_INPUT_REG           0x04
#define RAUX2_INPUT_REG           0x05
#define L_DAC_OUTPUT_REG          0x06
#define R_DAC_OUTPUT_REG          0x07
#define FS_PLAYBACK_DATA_REG      0x08
#define INTERFACE_REG             0x09

#define CEN                       0x02
#define PEN                       0x01
#define AUTOCAL                   0x08
#define PLAYPIO                   0x40
#define CAPPIO                    0x80

#define PIN_CNTL_REG              0x0a
#define ERR_STATUS_INIT_REG       0x0b
#define MODE_ID_REG               0x0c
#define LOOPBACK_CNTL_REG         0x0d
#define PLAYBACK_UPCOUNT_REG      0x0e
#define PLAYBACK_LOCOUNT_REG      0x0f
#define ALT_FEATURE_1_REG         0x10
#define ALT_FEATURE_2_REG         0x11
#define L_LINE_INPUT_REG          0x12
#define R_LINE_INPUT_REG          0x13
#define TIMER_LOBYTE_REG          0x14
#define TIMER_HIBYTE_REG          0x15

#define ALT_FEATURE_STAT_REG      0x18
#define   PLAYBACK_INTR           0x10
#define   RECORD_INTR             0x20

#define VERSION_CHIP_ID_REG       0x19
#define MONO_IO_CNTL_REG          0x1a
#define CAPTURE_DATA_REG          0x1c
#define CAPTURE_UPCOUNT_REG       0x1e
#define CAPTURE_LOCOUNT_REG       0x1f



/* Audio Setup Definitions
 * -----------------------                                                    */

#define MODE1                     0x00
#define MODE2                     0x40
#define XTAL1                     0x00
#define XTAL2                     0x20
#define MCE_SET                   0x40
#define TRD                       0x20
#define MUTE                      0x80
#define MONO_O_MUTE               0x40
#define MONO                      0x00
#define STEREO                    0x10
#define LINEAR_8                  0x00
#define LINEAR_16                 0x40
#define u_LAW                     0x20
#define A_LAW                     0x60
#define DISABLE                   0
#define MIC_INPUT                 0x80
#define MIC_20DB                  0x20

typedef enum _DATA_FORMAT  { L_8, L_16, u, A } DATA_FORMAT;

typedef enum _INPUT_SOURCE { LINE, AUX1, MIC, LPM } INPUT_SOURCE;

typedef enum _SAMPLE_RATE  { KHZ5o5152, KHZ6o615, KHZ8o0, KHZ9o6, KHZ11o025,
                             KHZ16o0, KHZ18o9, KHZ22o05, KHZ27o42857, KHZ32o0,
                             KHZ33o075, KHZ37o8, KHZ44o1, KHZ48o0 } SAMPLE_RATE;

typedef enum _TRANSFER_TYPE { CP_PIO, DUAL_DMA, SINGLE_DMA } TRANSFER_TYPE;

typedef enum _DMA_OPERATION { DMA_OFF, CAPTURE, PLAYBACK } DMA_OPERATION;

typedef enum _DMA_CYCLE { SINGLE_CYCLE, BURST_CYCLE } DMA_CYCLE;


/*
 *               S T R U C T U R E S
 */


typedef struct _AUDIO_REC {
        unsigned int        audio_mode;
        TRANSFER_TYPE       transfer_type;
        DMA_OPERATION       dma_operation;
        DMA_CYCLE           dma_cycle;
        int                 cycle;
        unsigned int        bytes;
        SAMPLE_RATE         sample_rate;
        unsigned char       clock;        /* Crystal clock */
        unsigned char       cfs;          /* Clock Frequency Divide Select */
        unsigned char       type;         /* Mono/Stereo type */
        DATA_FORMAT         data_format;
        unsigned char       data;         /* Data Format */
        INPUT_SOURCE        isource;
        unsigned char       l_input;      /* Left Input */
        unsigned char       r_input;      /* Right Input */
        unsigned char       l_20db;       /* Left Microphone 20dB gain enable */
        unsigned char       r_20db;       /* Right Microphone 20dB gain enable */
        unsigned char       in_gain;
        unsigned char       l_gain;       /* Left input gain */
        unsigned char       r_gain;       /* Right input gain */
        unsigned char       aux1_attn;
        unsigned char       l_aux1;       /* Left Auxiliary 1 attenuate */
        unsigned char       r_aux1;       /* Right Auxiliary 1 attenuate */
        unsigned char       out_attn;
        unsigned char       l_out;        /* Left output attenuate */
        unsigned char       r_out;        /* Right output attenuate */
        unsigned char       dme;          /* Digital Mix Enable */
        unsigned char       dm_attn;      /* Digital Mix Attenuate */
        unsigned char       hp_filter;
        unsigned char       line_in_gain;
        unsigned char       lline_in;
        unsigned char       rline_in;
        unsigned char       mono_in;
        unsigned char       mono_out;
        int                 playback_cycle;
} AUDIO_REC;

AUDIO_REC audio;
#if 0
BUD_DMA_SLAVE audio_dma;
BUD_DMA_SLAVE capture_dma;
BUD_DMA_SLAVE playback_dma;
#endif
int global_audio_dma;
unsigned int simult_operation;



typedef struct _RECORD_SIZE {
        unsigned int        start_addr;
        unsigned int        end_addr;
        unsigned int        size;
} RECORD_SIZE;


