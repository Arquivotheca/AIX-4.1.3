/* @(#)65	1.2  src/bos/usr/lpp/Unicode/wim/wim.h, cfgnls, bos411, 9428A410j 3/4/94 10:16:09  */
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_wim_h
#define	_wim_h

/************************************************************************
 * Include files
 ************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/id.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <im.h>
#include <iconv.h>
#include <langinfo.h>
#include <sys/param.h>

/* These are defined in param.h */
/* #define MIN(X,Y)  ((X > Y) ? (Y) : (X)) */
/* #define MAX(X,Y)  ((X > Y) ? (X) : (Y)) */

/* NOTE: The WIM is a shared object, and globals have to be handled with 
 * care. */

#define IMCFG_SUFFIX  ".imcfg"
#define LOCSEL_MODMASK (ControlMask | Mod1Mask)
#define CLSEL_MODMASK (ControlMask | Mod1Mask)
#define CSEL_MODMASK (ControlMask | Mod1Mask)
#define LOCSEL_XK (XK_i)
#define CLSEL_XK (XK_l)
#define CSEL_XK (XK_c)

#define MENU_PAGE_SIZE 10
#define MENU_DIGITS (((MENU_PAGE_SIZE - 1) / 10) + 1)
#define MENU_LINE_LEN 24
/* Add 2 lines for page-up and page-down. */
#define LINES_PER_PAGE (MENU_PAGE_SIZE + 2)

/* The data for a menu page. */
typedef struct _MenuPage {
    IMSTRATT lines[LINES_PER_PAGE];
    char strbuf[LINES_PER_PAGE][MENU_LINE_LEN];
    char attbuf[LINES_PER_PAGE][MENU_LINE_LEN];
} MenuPageRec;

/* NOTE: Keeping this at 1 will allow both the ScrollList and RowColumn
 * resource settings to work. */
#define CLIST_NUM_COLS 16

/* A menu structure, including a pointer to the data. */
typedef struct _Menu {
    IMAuxInfo auxinfo;
    char **items;
    uint_t num_choices;
    int supp_selection;
    uint_t num_cols;
    uint_t page;
    uint_t num_pages;
    MenuPageRec *pages;
} MenuRec;

#define MAX_CFG_LINE 128
#define MAX_CFG_FIELD 32

typedef struct _Crange {
    wchar_t first;
    wchar_t last;
} CrangeRec;

typedef struct _Clist {
    char label[MAX_CFG_FIELD];
    CrangeRec *ranges;
    uint_t numranges;
    char **chars;
    char *charbuf;
    MenuRec menu;
} ClistRec;

/* Default clist is for ASCII. */
CrangeRec ASCIIRangeRec = {0x0020, 0x007f};
ClistRec DefClistRec = {"ASCII", &ASCIIRangeRec, 1, NULL, NULL};

typedef struct _FepData {
    char description[MAX_CFG_FIELD];
    char locale[MAX_CFG_FIELD];
    char imlocale[MAX_CFG_FIELD];
    char code[MAX_CFG_FIELD];
    IMFepRec *fep;
    iconv_t cd;
} FepDataRec;

/* Default IM is for C locale */
FepDataRec DefFepRec = {"C", "C", "C", "IBM-850", NULL, NULL};

/* The FepTable is stored in the Fep, because it might need to be different
 * for each application or user. (NOTE: as in an X resource) */
typedef struct _WIMFep {
    IMFepCommon	common;		/* IMFEP common */
    FepDataRec *feptbl;
    uint_t numlocs;
    char **loclabels;
    uint_t defloc;		/* Last previously selected locale. */
    MenuRec locmenu;		/* Original FEP copy of menu. */
    ClistRec *clisttbl;
    ClistRec *clist;		/* Current clist. */
    char **clistlabels;
    uint_t numclists;
    MenuRec clistmenu;		/* Original FEP copy of menu. */
    uint_t locsel_modmask;
    uint_t locsel_xk;
    uint_t clsel_modmask;
    uint_t clsel_xk;
    uint_t csel_modmask;
    uint_t csel_xk;
} WIMFepRec;

typedef struct _ObjData {
    IMObjectRec *obj;
} ObjDataRec;

#define MAX_LOC_NAME 128

typedef struct _WIMObject	{
    IMObjectCommon common;	/* IM Common info */
    IMSTR str;		        /* String used for conversion, etc. */
    uint_t str_siz;
    IMSTR filtstr;	        /* String returned by WIMFilter, or 
				 * WIMLookup. */
    uint_t filtstr_siz;
    IMIndicatorInfo indinfo;	/* Argument to callback -- Not used. */
    IMSTR indstr;		/* WIM indicator string. */
    uint_t indstr_siz;
    IMTextInfo textinfo;
    uint_t textinfo_siz;
    int textinfo_space;
    IMAuxInfo auxinfo;
    uint_t auxinfo_siz;		/* Size of buffers in auxinfo. */
    uint_t auxinfo_nline;	/* Number of lines in message area. */
    uint_t auxinfo_nitem;	/* Number of items in selection area. */
    IMCallback wimcb;		/* The WIM's callbacks (called by child IM) */
    ObjDataRec *objtbl;
    uint_t numlocs;
    uint_t curloc;		/* current locale (child IM). */
    caddr_t auxid;		/* auxid (only one is allowed per object?). */
    MenuRec locmenu;		/* copy of FEP menu structure. */
    MenuRec clistmenu;		/* copy of FEP menu structure. */
    uint_t state;
    int supp_selection;
    MenuRec *curmenu;		/* currently active menu (not necessarily up */
} WIMObjectRec;

#define BASE_STATE	0
#define LOCSEL_STATE	0x00000001
#define CLSEL_STATE 	0x00000002
#define CSEL_STATE	0x00000004
#define WIM_STATE	0x00000007 /* Combination of all WIM states. */
#define CHILDAUX_STATE	0x00010000
#define CHILD_STATE	0x00010000 /* Combination of all child states. */

/* WIM Callbacks */
static int WIMTextDraw(IMObjectRec *obj, IMTextInfo *textinfo, caddr_t udata);
static int WIMTextHide(IMObjectRec *obj, caddr_t udata);
static int WIMTextStart(IMObjectRec *obj, int *space, caddr_t udata);
static int WIMTextCursor(IMObjectRec *obj, uint_t direction, int *cur, 
    caddr_t udata);
static int WIMAuxCreate(IMObjectRec *obj, caddr_t *auxidp, caddr_t udata);
static int WIMAuxDraw(IMObjectRec *obj, caddr_t auxid, IMAuxInfo *auxinfo,
	caddr_t udata);
static int WIMAuxHide(IMObjectRec *obj, caddr_t auxid, caddr_t udata);
static int WIMAuxDestroy(IMObjectRec *obj, caddr_t auxid, caddr_t udata);
static int WIMIndicatorDraw(IMObjectRec *obj, IMIndicatorInfo *indicatorinfo, 
	caddr_t udata);
static int WIMIndicatorHide(IMObjectRec *obj, caddr_t udata);
static int WIMBeep(IMObjectRec *obj, int percent, caddr_t udata);

/* WIM IMFep Functions */
WIMFepRec *WIMInitialize(IMLanguage language);
static WIMObjectRec *WIMCreate(WIMFepRec *fep, IMCallback *cb, caddr_t udata);
static void WIMDestroy(WIMObjectRec *obj);
static int WIMFilter(WIMObjectRec *obj, uint_t keysym, uint_t state,
    caddr_t *str, uint_t *len);
static int WIMLookup(WIMObjectRec *obj, uint_t keysym, uint_t state,
	caddr_t *str, uint_t *len);
static int WIMProcess(WIMObjectRec *obj, uint_t keysym, uint_t state,
	caddr_t *str, uint_t *len);
static int WIMProcessAuxiliary(WIMObjectRec *obj, caddr_t auxid, uint_t button,
	uint_t panel_row, uint_t panel_col, uint_t item_row, uint_t item_col,
	caddr_t *str, uint_t *str_len);
static int WIMIoctl(WIMObjectRec *obj, int cmd, caddr_t arg);
static void WIMClose(WIMFepRec *fep);

/* Ioctl Handling Functions */
static int WIMRefresh(WIMObjectRec *obj, caddr_t arg, ObjDataRec *orec, 
	FepDataRec *frec);
static int WIMQueryIndicatorString(WIMObjectRec *obj, 
	IMQueryIndicatorString *qindstr, ObjDataRec *orec, FepDataRec *frec);
static int WIMQueryAuxiliary(WIMObjectRec *obj, IMQueryAuxiliary *qaux,
	ObjDataRec *orec, FepDataRec *frec);

/* iconv() Conversion Functions */
static int iconv_str(iconv_t cd, IMSTR *istr, IMSTR *ostr, uint_t *osiz);
static int iconv_stratt(iconv_t cd, IMSTRATT *istratt, IMSTRATT *ostratt, 
        int *osiz);
static int iconv_len(iconv_t cd, uchar_t *istr, uint_t ilen, uint_t *olen);
static int iconv_textinfo(iconv_t cd, IMTextInfo *itext, IMTextInfo *otext, 
	int *osiz);
static int iconv_auxinfo(iconv_t cd, IMAuxInfo *iaux, IMAuxInfo *oaux, 
	int *osiz, int *online, int *onitem);

/* Preedit manipulation functions */
static int preedit_init(WIMObjectRec *obj, ObjDataRec *orec);
static int preedit_commit(WIMObjectRec *obj, IMSTR *str, uint_t *str_siz,
    ObjDataRec *orec);

/* Menu manipulation functions */
static int menu_init(MenuRec *rec, char *title, uint_t num_cols);
static int menu_supp_selection(MenuRec *rec, int supp_selection);
static int menu_fill(MenuRec *rec, char **labels, uint_t num_labels);
static int menu_clear(MenuRec *rec);
static int menu_process_auxiliary(MenuRec *rec, uint_t button,
	uint_t panel_row, uint_t panel_col, uint_t item_row, 
	uint_t item_col, int *selection);
static int menu_filter(MenuRec *rec, uint_t keysym, uint_t state, 
    WIMFepRec *wfep, WIMObjectRec *wobj, IMObject *obj, int *selection);
static int menu_reorder(MenuRec *rec, uint_t selection);
static int menu_hide(MenuRec *rec, WIMFepRec *wfep, WIMObjectRec *wobj, 
    IMObject *obj);
static int menu_show(MenuRec *rec, WIMFepRec *wfep, WIMObjectRec *wobj, 
    IMObject *obj);

/* Miscellaneous Functions */
static int imstr_copy(char *str, uint_t len, IMSTR *ostr, uint_t *osiz);
static int imstr_append(char *str, uint_t len, IMSTR *ostr, uint_t *osiz);
static int imstratt_copy(char *str, uint_t len, char att, 
    IMSTRATT *ostr, uint_t *osiz);
static int imstratt_append(char *str, uint_t len, char att, 
    IMSTRATT *ostr, uint_t *osiz);
static int find_loc(WIMFepRec *fep, char *language);
static int get_data_recs(WIMObjectRec *obj, FepDataRec **frecp,
    ObjDataRec **orecp);
static int wim_filter(WIMObjectRec *obj, uint_t keysym, uint_t state,
    IMSTR *str, uint_t *str_siz, ObjDataRec *orec, FepDataRec *frec);
static int *get_locpath(IMLanguage language, char *suffix, char* pathname);
static int read_config(WIMFepRec *fep, char *path);
static int parse_locline(WIMFepRec *fep, char *line);
static int parse_clistline(WIMFepRec *fep, char *line);
static int menu_init(MenuRec *rec, char *title, uint_t num_cols);
static int menu_supp_selection(MenuRec *rec, int supp_selection);
static int clist_fill(ClistRec *clist, int supp_selection);

#endif	/* _wim_h */
