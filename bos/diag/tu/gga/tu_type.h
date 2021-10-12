/* @(#)92	1.1  src/bos/diag/tu/gga/tu_type.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:33 */
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: DEBUG_MSG
 *              LOG_ERROR
 *              LOG_MSG
 *              LOG_SYSERR
 *              TITLE
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
 */


#ifdef TITLES
#define TITLE(msg)   printf ("%s\n", msg);

#elif LOG_TRACES
/* the application must open/close the output file : logfile */
static int logfile;
#define TITLE(msg) { fprintf (logfile, "%s\n", msg);  fflush (logfile);      \
                   }

#else
#define TITLE(msg)
#endif

/* Define LOGMSGS if you want messages to be logged                          */

#ifdef LOGMSGS
extern void logmsg (char *);
extern void logerror (char *);
extern void log_syserr (char *);
extern void update_msg (void);
#define LOG_SYSERR(msg)      log_syserr (msg)
#define LOG_MSG(msg)         logmsg (msg)
#define LOG_ERROR(msg)       logerror (msg)
#define ALIVE_MSG            update_msg ()
#else
#define LOG_SYSERR(msg)
#define LOG_MSG(msg)
#define LOG_ERROR(msg)
#define ALIVE_MSG
#endif

#ifdef DEBUG_GGA
#define DEBUG_MSG(msg)  printf("\n***  %s  ***", msg);fflush(stdout)
#else
#define DEBUG_MSG(msg)
#endif


typedef unsigned char  BOOL;


/* Maximum resolutions :                                                     */
#define MAX_NUM_COLS               1280          /* X axis in pixels         */
#define MAX_NUM_SCANS              1024          /* Y axis in pixels         */
#define MAX_PHYSICAL_SCAN_LINES    1024          /* no. of scan lines of VRAM*/
#define MAX_PHYSICAL_PIX_PER_LINE  2048          /* no. of pixels of VRAM    */
#define PAL_LINES                  256     /*Palette RAM (256 x 24 bit words)*/
#define BPP                        8


/* If keytouch function is not enabled, next define gives the time a         */
/* displayed pattern stays on the screen.                                    */

#define DEFAULT_DPLY_TIME         0                            /* in seconds */
#define DECAY_MEMORY_TIME         1            /* second, decay memory tests */

/* use next defines with enable_int() and disable_int() functions            */
#define WTKN_INT   ((unsigned char) 0)
#define ERROR_INT  ((unsigned char) 1)

/* All GGA PCI registers                                                     */
enum
{
  VENDOR_ID_REG = 0, DEVICE_ID_REG, COMMAND_REG, DEVICE_STATUS_REG,
  REVISION_ID_REG, STD_PROG_INT_REG, SUBCLASS_CODE_REG, CLASS_CODE_REG,
  CACHE_LINE_SIZE_REG, LATENCY_TIMER_REG, HEADER_TYPE, BIST_CONTROL_REG,
  BUFFER_BASE_ADDR_REG, INTERRUPT_LINE_REG, INTERRUPT_PIN_REG,
  MIN_GRANT_REG, MAX_GRANT_REG,

  PCI_REG_NUM
};


/* All W9100 memory mapped registers                                         */
enum
{
  SYSCONFIG_REG = 0, INTERRUPT_REG, INTERRUPT_ENBL_REG,

  STATUS_REG, BLIT_CMD_REG, QUAD_CMD_REG,
  CINDEX_REG, W_OFFSET_REG,

  FGROUND_REG, BGROUND_REG, COLOR1_REG, COLOR0_REG, PLANE_MASK_REG, RASTER_REG,
  PIXEL1_REG, PIXEL8_REG,
  W_P_MIN_XY_REG, W_P_MAX_XY_REG,
  W_B_MIN_XY_REG, W_B_MAX_XY_REG,

  HRZC_REG, HRZT_REG, HRZSR_REG, HRZBR_REG, HRZBF_REG, PREHRZC_REG, VRTC_REG,
  VRTT_REG, VRTSR_REG, VRTBR_REG, VRTBF_REG, PREVRTC_REG, SRADDR_REG, SRTCTL_REG,
  SRADDR_INC_REG, SRTCTL2_REG,

  MEMCNFG_REG, RFPER_REG, RLMAX_REG, PU_CONFIG_REG,

  IGC_REG_NUM
};

typedef unsigned long IGC_REGS_TYPE [IGC_REG_NUM];

enum
{
  CURCOL_TRANS_COLOR = 0, CURCOL1, CURCOL2, CURCOL3,
  NUM_CURSOR_COLORS
};


enum
{
  VIDEO_CTL_REG_FMT = 0, USER_REG_FMT, PIXEL_FMT, RAMDAC_FMT,
  NON_WTKN_FMT
};

enum COLOR_PALETTE
{
  BLACK = 0, RED, GREEN, YELLOW, BLUE, MAGENTA, BABY_BLUE, LIGHT_GRAY, GRAY,
  ORANGE, DARK_GREEN, BROWN, LIGHT_BLUE, LIGHT_MAGENTA, CYAN, WHITE, BRICK_RED,
  RUST, PINK, BEIGE, OLIVE_GREEN, SALMON, OFF_WHITE, LIGHT_GREEN, SKY_BLUE,
  PALE_PINK, DARK_BROWN,
  NUM_COLORS
};

enum  { PIXEL1_CMD, PIXEL8_CMD };

typedef struct
{
  float   xstart, ystart;
  float   xend, yend;
  unsigned char color;
} BOX;


/* next struct. used to draw a quadrilateral */
typedef struct
{
  float   x[4], y[4];                            /* 4 vertices for quad cmd */
  unsigned char color;
} QUAD;


/* next struct. used to draw a triangle */
typedef struct
{
  float   x[3], y[3];                            /* 3 vertices for triangle  */
  unsigned char color;
} TRIANGLE;


/* next struct. used to draw a point */
typedef struct
{
  float   x, y;                                  /* x & y coordinate of point*/
  unsigned char color;
} POINT;


/* next struct. used to draw a line */
typedef struct
{
  float   x1, y1, x2, y2;                        /* x & y coordinate of line */
  unsigned char color;
} LINE;


#define BIT_MAP_SIZE                      5      /* 5 words per character    */
#define CHARACTER_WIDTH_IN_PIXEL          8    /* width of character in bits */
#define CHARACTER_HEIGHT_IN_PIXEL         20   /* height of char in bits     */

typedef unsigned long CHAR_BITMAP [BIT_MAP_SIZE];



/* ------------------------------------------------------------------------- */
/* Function prototypes                                                       */
/* ------------------------------------------------------------------------- */

extern void syserr (char *);
extern BOOL in_mom (void);
extern int enter_mom (void);
extern int exit_mom (void);
extern void reset_grant (void);
extern BOOL granted (void);
extern BOOL write_igc_reg (unsigned char, unsigned long);
extern BOOL get_igc_reg(unsigned char, unsigned long *);
extern void igc_write_2D_meta_coord (unsigned long, unsigned long, unsigned long, unsigned long);
extern void igc_write_2D_coord_reg (unsigned long, unsigned long, unsigned long, unsigned long);
extern void igc_write_pixel1 (unsigned char, unsigned long);
extern void igc_write_pixel8 (unsigned long);
extern void igc_do_blit (void);
extern void igc_draw_quad (void);
extern void save_regs (void);
extern void restore_regs (void);
extern void disable_video (void);
extern void enable_video (void);
extern int clear_screen (void);
extern BOOL disable_int (unsigned char);
extern GGAPARM *get_input_parm (void);
extern void set_tu_errno (void);
extern void set_dply_time (unsigned long);
extern unsigned long get_dply_time (void);
extern int set_op_mode (void);
extern char *get_tu_name (unsigned long);
extern void set_foreground_color (unsigned long);
extern void set_background_color (unsigned long);
extern void set_color_1 (unsigned long);
extern void set_color_0 (unsigned long);
extern void get_screen_res (unsigned long *, unsigned long *);
extern int draw_box (BOX *);
extern int draw_quad (QUAD *);
extern int draw_triangle (TRIANGLE *);
extern int draw_point (POINT *);
extern int draw_line (LINE *);
extern int draw_circle (unsigned long, unsigned long, unsigned long, unsigned char, float);
extern void fill_palette (unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
extern void gga_initialize (void);
extern void reset_palette (void);
extern unsigned char get_cursor_status (void);
extern void disable_cursor(void);
extern void enable_cursor(void);
extern void draw_rgb_cursor (void);
extern int set_cursor_pos(unsigned long, unsigned long);
extern void get_cursor_pos(unsigned long *, unsigned long *, unsigned long);
extern void set_wclip (unsigned long, unsigned long, unsigned long, unsigned long);
extern void set_w_offset (unsigned long, unsigned long);
extern void initialize_reg_attr (void);
extern void wait_for_wtkn_ready (void);
extern void color_init (void);
extern int full_color (unsigned char, BOOL);
extern unsigned long get_full_xy(unsigned long);
extern unsigned long get_part_xy (unsigned long, unsigned long);
extern unsigned short get_monitor_type (void);
extern void set_mem_info (unsigned long, unsigned long, unsigned long, int, char *, char *);
extern void update_mem_info (MEM *);
extern unsigned long get_color_pattern (unsigned long color);
extern void draw_character (CHAR_BITMAP, unsigned char, unsigned char, unsigned char, unsigned long);
extern void set_new_position (unsigned long *, unsigned long *, unsigned long *, unsigned long *,
                              unsigned long, unsigned long, unsigned char, unsigned char, unsigned long);
extern void dump_gga_regs (void);
extern void dump_wtk_sys_cntl_regs (void);
extern void dump_wtk_param_eng_regs (void);
extern void dump_wtk_draw_eng_regs (void);
extern void dump_wtk_video_cntl_regs (void);
extern void dump_BT485_regs (void);
extern void WriteIBM525(unsigned short, unsigned char);
extern unsigned char ReadIBM525(unsigned short);
extern unsigned long get_end_of_frame_buffer(void);
extern unsigned int modeset(int);
extern void  out32le(unsigned long, unsigned long);
extern void  out32(unsigned long, unsigned long); 
extern void  out8(unsigned long, unsigned char);  
extern void  BR_WL(unsigned long, unsigned long); 
extern void  WL(unsigned long, unsigned long);    
extern void  WC(unsigned char, unsigned long);    
extern unsigned long in32le(unsigned long);      
extern unsigned long in32(unsigned long);        
extern unsigned char in8(unsigned long);         
extern unsigned long BR_RL(unsigned long);       
extern unsigned long RL(unsigned long);          
extern unsigned char RC(unsigned long);          
extern void Debug_Brkpt(void);

#ifdef DISP_MSG
extern void display_message (unsigned long, unsigned long, unsigned char, unsigned char,
                             CHAR_BITMAP *, unsigned char, int);
#endif

extern void restore_default_colors (void);
extern void save_default_colors (void);

/* ------------------------------------------------------------------------- */
/* TUs :                                                                     */
/* ------------------------------------------------------------------------- */

extern int pci_tu (void);
extern int reg_tu (void);
extern int vram_tu (void);
extern int palette_tu (void);
extern int palettecheck_tu (void);
extern int cursor_ram_tu (void);
extern int black_tu (void);
extern int white_tu (void);
extern int red_tu (void);
extern int green_tu (void);
extern int blue_tu (void);
extern int colorbar_tu (void);
extern int wb9x9_tu (void);
extern int wb9x11_tu (void);
extern int sqr_box_50mm_tu (void);
extern int string_test_tu (void);
extern int tu_open(char *);
extern int tu_close(void);
extern int pixel8_str_tst_tu (void);
extern int video_tu (void);

#ifdef ALL_TUS
extern int advanced_dply_tu (void);
extern int blit_tu (void);
extern int pixel1_tu (void);
extern int pixel8_tu (void);
extern int rgb_tu (void);
extern int dply_cursor_tu (void);
extern int bw64_tu (void);
extern int wb64_tu (void);
extern int bw128_tu (void);
extern int wb128_tu (void);
extern int bw160_tu (void);
extern int wb160_tu (void);
extern int bw9x11_dots_tu (void);
extern int wb9x11_dots_tu (void);
extern int scroll_h_gmode_tu0 (void);
extern int scroll_h_gmode_tu1 (void);
extern int scroll_h_draw (void);
extern int scroll_h_pixel1 (void);
extern int scroll_h_pixel1_emc (void);
extern int luminance_fall_off_tu (void);
extern int luminance_tu_1 (void);
extern int luminance_tu_2 (void);
extern int luminance_tu_3 (void);
extern int luminance_tu_4 (void);
extern int luminance_tu_5 (void);
extern int luminance_tu_6 (void);
extern int luminance_tu_7 (void);
extern int luminance_tu_8 (void);
extern int luminance_tu_9 (void);
extern int luminance_tu_10 (void);
extern int luminance_tu_11 (void);
extern int luminance_tu_12 (void);
#endif

