/* @(#)35       1.4  src/bos/diag/tu/wga/tu_type.h, tu_wga, bos411, 9428A410j 1/3/94 17:32:56 */
/*
 *   COMPONENT_NAME: TU_WGA (TU header file)
 *
 *   FUNCTIONS: DEBUG_MSG
 *              LOG_ERROR
 *              LOG_MSG
 *              LOG_SYSERR
 *              TITLE
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
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

#ifdef DEBUG_WGA
#define DEBUG_MSG(msg)  printf("\n***  %s  ***", msg);fflush(stdout)
#else
#define DEBUG_MSG(msg)
#endif


typedef uchar_t  BOOL;


/* Maximum resolutions :                                                     */
#define MAX_NUM_COLS               1280          /* X axis in pixels         */
#define MAX_NUM_SCANS              1024          /* Y axis in pixels         */
#define MAX_PHYSICAL_SCAN_LINES    1024          /* no. of scan lines of VRAM*/
#define MAX_PHYSICAL_PIX_PER_LINE  2048          /* no. of pixels of VRAM    */
#define PAL_LINES                  256     /*Palette RAM (256 x 24 bit words)*/
#define BPP                        8


/* The time a displayed pattern stays on the screen       */

#define DEFAULT_DPLY_TIME         0                            /* in seconds */
#define DECAY_MEMORY_TIME         1            /* second, decay memory tests */

/* use next defines with enable_int() and disable_int() functions            */
#define WTKN_INT   ((uchar_t) 0)
#define ERROR_INT  ((uchar_t) 1)


/* All registers for BUID 40 address map                                     */
enum
{
  ADCNTL_REG = 0, ADSTAT_REG, WORIG_REG, VPD0_REG, VPD1_REG, VIDEO_ROM_REG,
  ERRADDR_REG,
  WGA_REG_NUM
};

typedef ulong_t WGA_REGS_TYPE [WGA_REG_NUM];


/* All registers for WTKN  Integrated Graphics Crontroller                   */
enum
{
  HRZC_REG = 0, HRZT_REG, HRZSR_REG, HRZBR_REG, HRZBF_REG, PREHRZC_REG,
  VRTC_REG, VRTT_REG, VRTSR_REG, VRTBR_REG, VRTBF_REG, PREVRTC_REG,
  SRADDR_REG, SRTCTL_REG,
  DAC_ADL_REG, DAC_ADH_REG, COLOR_PALETTE_RAM,
  USER_REG, STATUS_REG,
  CINDEX_REG, W_OFFSET_REG,
  FGROUND_REG, BGROUND_REG, RASTER_REG, PLANE_MASK_REG,
  W_MIN_XY_REG, W_MAX_XY_REG,
  PIXEL1_REG, PIXEL8_REG,

  INTERRUPT_REG, INTERRUPT_ENBL_REG,

  CURCMND_REG,
  IGC_REG_NUM
};

typedef ulong_t IGC_REGS_TYPE [IGC_REG_NUM];

enum
{
  CURSOR_TRANS_COLOR, CURCOL1_REG, CURCOL2_REG, CURCOL3_REG,
  NUM_CURSOR_COLOR_REGS
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
  uchar_t color;
} BOX;


/* next struct. used to draw a quadrilateral */
typedef struct
{
  float   x[4], y[4];                            /* 4 vertices for quad cmd */
  uchar_t color;
} QUAD;


/* next struct. used to draw a triangle */
typedef struct
{
  float   x[3], y[3];                            /* 3 vertices for triangle  */
  uchar_t color;
} TRIANGLE;


/* next struct. used to draw a point */
typedef struct
{
  float   x, y;                                  /* x & y coordinate of point*/
  uchar_t color;
} POINT;


/* next struct. used to draw a line */
typedef struct
{
  float   x1, y1, x2, y2;                        /* x & y coordinate of line */
  uchar_t color;
} LINE;


#define BIT_MAP_SIZE                      5      /* 5 words per character    */
#define CHARACTER_WIDTH_IN_PIXEL          8    /* width of character in bits */
#define CHARACTER_HEIGHT_IN_PIXEL         20   /* height of char in bits     */

typedef ulong_t CHAR_BITMAP [BIT_MAP_SIZE];



/* ------------------------------------------------------------------------- */
/* Function prototypes                                                       */
/* ------------------------------------------------------------------------- */

extern void syserr (char *);
extern BOOL in_graphics (void);
extern int enter_graphics(void);
extern int exit_graphics (void);

extern void get_all_wga_regs (WGA_REGS_TYPE);
extern BOOL get_wga_reg (ulong_t *, uchar_t);
extern BOOL check_wga_error (int *);
extern BOOL write_wga_reg (ulong_t, uchar_t);
extern BOOL write_igc_reg (uchar_t, ulong_t);
extern BOOL get_igc_reg(uchar_t, ulong_t *);
extern void igc_write_2D_meta_coord (ulong_t, ulong_t, ulong_t, ulong_t);
extern void igc_write_2D_coord_reg (ulong_t, ulong_t, ulong_t, ulong_t);
extern void igc_write_pixel1 (uchar_t, ulong_t);
extern void igc_write_pixel8 (ulong_t);
extern void igc_do_blit (void);
extern void igc_draw_quad (void);
extern void save_regs (void);
extern void restore_regs (void);
extern void disable_video (void);
extern void enable_video (void);
extern int clear_screen (void);
extern BOOL disable_int (uchar_t);
extern BOOL enable_int (uchar_t);
extern WGAPARM *get_input_parm (void);
extern void set_tu_errno (void);
extern int reset_wga_error (void);
extern void set_dply_time (ulong_t);
extern ulong_t get_dply_time (void);
extern int set_op_mode (void);
extern char *get_tu_name (ulong_t);
extern void set_foreground_color (uchar_t);
extern void set_background_color (uchar_t);
extern void get_screen_res (ulong_t *, ulong_t *);
extern int draw_box (BOX *);
extern int draw_quad (QUAD *);
extern int draw_triangle (TRIANGLE *);
extern int draw_point (POINT *);
extern int draw_line (LINE *);
extern int draw_circle (ulong_t, ulong_t, ulong_t, uchar_t, float);
extern void fill_palette (ulong_t, ulong_t, ulong_t, uchar_t, uchar_t, BOOL);
extern void wga_initialize (void);
extern void reset_palette (void);
extern ulong_t get_cursor_status (void);
extern void disable_cursor(void);
extern void enable_cursor(void);
extern void draw_rgb_cursor (void);
extern ulong_t get_cursor_color(ulong_t);
extern int set_cursor_pos(ulong_t, ulong_t);
extern void get_cursor_pos(ulong_t *, ulong_t *);
extern void set_wclip (ulong_t, ulong_t, ulong_t, ulong_t);
extern void set_w_offset (ulong_t, ulong_t);
extern void initialize_reg_attr (void);
extern void wait_for_wtkn_ready (void);
extern void load_dac_reg (ulong_t);
extern void color_init (void);
extern int full_color (uchar_t);
extern int sign_extend (ulong_t, uchar_t);
extern ulong_t get_full_xy(ulong_t);
extern ulong_t get_part_xy (ulong_t, ulong_t);
extern ulong_t get_monitor_type (void);
extern void set_mem_info (ulong_t, ulong_t, ulong_t, int, char *, char *);
extern void update_mem_info (MEM *);
extern ulong_t get_color_pattern (ulong_t color);
extern void draw_character (CHAR_BITMAP, uchar_t, uchar_t, uchar_t, ulong_t);
extern void draw_AT_character (ulong_t [], uchar_t, uchar_t);
extern void set_new_position (ulong_t *, ulong_t *, ulong_t *, ulong_t *,
                              ulong_t, ulong_t, uchar_t, uchar_t, ulong_t);
#ifdef DISP_MSG
extern void display_message (ulong_t, ulong_t, uchar_t, uchar_t,
                             CHAR_BITMAP *, uchar_t, int);
#endif

#ifdef TRASH
extern void restore_default_colors (void);
#endif


/* ------------------------------------------------------------------------- */
/* TUs :                                                                     */
/* ------------------------------------------------------------------------- */

extern int vpd_tu (void);
extern int reg_tu (void);
extern int vram_tu (void);
extern int palette_tu (void);
extern int cursor_ram_tu (void);
extern int interrupt_tu (void);
extern int video_rom_scan_tu (void);
extern int tu_close(void);
extern int tu_open(void);
extern int black_tu (void);
extern int white_tu (void);
extern int red_tu (void);
extern int green_tu (void);
extern int blue_tu (void);
extern int colorbar_tu (void);
extern int wb9x9_tu (void);
extern int wb9x11_tu (void);
extern int sqr_box_50mm_tu (void);
extern int display_AT_tu (void);
extern int string_test_tu (void);
extern int fast_copy_tu (void);
extern int pixel8_str_tst_tu (void);

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
